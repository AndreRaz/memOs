#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "memory.h"
#include "commands.h"
#include "output.h"

/* ── remember ────────────────────────────────────────────
   Uso: mnemos remember "texto" [--type episodic|semantic|procedural]
                                [--importance 0.0-1.0]
*/
void mem_remember(sqlite3 *db, int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Uso: mnemos remember \"texto\" [--type TYPE] [--importance N]\n");
        return;
    }

    const char *content    = argv[0];
    const char *type       = "semantic";   /* default */
    double      importance = 0.5;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--type") == 0)
            type = argv[i+1];
        else if (strcmp(argv[i], "--importance") == 0)
            importance = atof(argv[i+1]);
    }

    /* Validar tipo */
    if (strcmp(type,"episodic")   != 0 &&
        strcmp(type,"semantic")   != 0 &&
        strcmp(type,"procedural") != 0) {
        fprintf(stderr, "Tipo inválido: %s (usa episodic, semantic o procedural)\n", type);
        return;
    }

    int id = memory_insert(db, content, type, importance);
    if (id > 0)
        printf("✓ Memoria guardada [id=%d, tipo=%s, importancia=%.1f]\n",
               id, type, importance);
}

/* ── log ─────────────────────────────────────────────────
   Uso: mnemos log --session ID "mensaje" --role user|assistant
*/
void mem_log(sqlite3 *db, int argc, char *argv[]) {
    const char *session_id = "default";
    const char *role       = "user";
    const char *content    = NULL;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--session") == 0 && i+1 < argc)
            session_id = argv[++i];
        else if (strcmp(argv[i], "--role") == 0 && i+1 < argc)
            role = argv[++i];
        else if (argv[i][0] != '-')
            content = argv[i];
    }

    if (!content) {
        fprintf(stderr, "Uso: mnemos log \"mensaje\" [--session ID] [--role user|assistant]\n");
        return;
    }

    /* Crear sesión si no existe */
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "INSERT OR IGNORE INTO sessions (id) VALUES (?);", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    /* Insertar mensaje */
    sqlite3_prepare_v2(db,
        "INSERT INTO messages (session_id, role, content) VALUES (?,?,?);",
        -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, role,       -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content,    -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("✓ Mensaje registrado [sesión=%s, rol=%s]\n", session_id, role);
    sqlite3_finalize(stmt);
}

/* ── list ────────────────────────────────────────────────
   Uso: mnemos list [--type TYPE]
*/
void mem_list(sqlite3 *db, int argc, char *argv[]) {
    const char *type = "";
    for (int i = 0; i < argc - 1; i++)
        if (strcmp(argv[i], "--type") == 0)
            type = argv[i+1];

    int n = memory_list(db, type);
    printf("  Total: %d memoria(s)\n\n", n);
}

/* ── recall ──────────────────────────────────────────────
   Uso: mnemos recall "consulta"
   Por ahora: full-text search con FTS5. Fase 2: cosine similarity.
*/
void mem_recall(sqlite3 *db, int argc, char *argv[]) {
    if (argc < 1) {
        fprintf(stderr, "Uso: mnemos recall \"consulta\"\n");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT m.id, m.type, m.decay, m.content "
        "FROM memories m "
        "JOIN memories_fts f ON m.id = f.rowid "
        "WHERE memories_fts MATCH ? "
        "ORDER BY m.decay DESC "
        "LIMIT 5;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error FTS: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, argv[0], -1, SQLITE_STATIC);

    printf("\nResultados para \"%s\":\n\n", argv[0]);
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int    id    = sqlite3_column_int(stmt, 0);
        const char *t = (const char *)sqlite3_column_text(stmt, 1);
        double decay  = sqlite3_column_double(stmt, 2);
        const char *c = (const char *)sqlite3_column_text(stmt, 3);
        output_memory_row(id, t, decay, c);
        count++;
    }

    if (count == 0) printf("  (sin resultados)\n");
    printf("\n");
    sqlite3_finalize(stmt);
}

/* ── forget ──────────────────────────────────────────────
   Uso: mnemos forget <id>
*/
void mem_forget(sqlite3 *db, int argc, char *argv[]) {
    if (argc < 1) { fprintf(stderr, "Uso: mnemos forget <id>\n"); return; }
    int id = atoi(argv[0]);
    if (memory_delete(db, id))
        printf("✓ Memoria [id=%d] eliminada.\n", id);
    else
        fprintf(stderr, "No se encontró memoria con id=%d\n", id);
}

/* ── stats ───────────────────────────────────────────────
   Uso: mnemos stats
*/
void mem_stats(sqlite3 *db) {
    printf("\n═══════════════════════════════\n");
    printf("  mnemOS — estadísticas\n");
    printf("═══════════════════════════════\n\n");

    sqlite3_stmt *stmt;
    /* Totales por tipo */
    sqlite3_prepare_v2(db,
        "SELECT type, COUNT(*), AVG(decay), AVG(importance) "
        "FROM memories GROUP BY type;",
        -1, &stmt, NULL);

    printf("  %-12s  %-6s  %-8s  %-8s\n",
           "tipo","total","decay","import.");
    printf("  %-12s  %-6s  %-8s  %-8s\n",
           "------------","------","--------","--------");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("  %-12s  %-6d  %-8.2f  %-8.2f\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_int(stmt,  1),
               sqlite3_column_double(stmt, 2),
               sqlite3_column_double(stmt, 3));
    }
    sqlite3_finalize(stmt);

    /* Sesiones */
    sqlite3_prepare_v2(db,
        "SELECT COUNT(*) FROM sessions;", -1, &stmt, NULL);
    sqlite3_step(stmt);
    printf("\n  Sesiones registradas: %d\n", sqlite3_column_int(stmt, 0));
    sqlite3_finalize(stmt);

    /* Mensajes */
    sqlite3_prepare_v2(db,
        "SELECT COUNT(*) FROM messages;", -1, &stmt, NULL);
    sqlite3_step(stmt);
    printf("  Mensajes en log:      %d\n\n", sqlite3_column_int(stmt, 0));
    sqlite3_finalize(stmt);
}