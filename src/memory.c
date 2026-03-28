#include <stdio.h>
#include <string.h>
#include "db.h"
#include "memory.h"
#include "output.h"

int memory_insert(sqlite3 *db, const char *content,
                  const char *type, double importance) {
    sqlite3_stmt *stmt;
    const char *sql =
        "INSERT INTO memories (content, type, importance) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparando insert: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, content,    -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, type,       -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, importance);

    int ok = sqlite3_step(stmt) == SQLITE_DONE;
    int id = ok ? (int)sqlite3_last_insert_rowid(db) : -1;

    sqlite3_finalize(stmt);
    return id;
}

int memory_list(sqlite3 *db, const char *type) {
    sqlite3_stmt *stmt;
    char sql[256];

    if (type && strlen(type) > 0) {
        snprintf(sql, sizeof(sql),
            "SELECT id, type, decay, content FROM memories "
            "WHERE type = ? ORDER BY decay DESC, last_used DESC;");
    } else {
        snprintf(sql, sizeof(sql),
            "SELECT id, type, decay, content FROM memories "
            "ORDER BY decay DESC, last_used DESC;");
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    if (type && strlen(type) > 0)
        sqlite3_bind_text(stmt, 1, type, -1, SQLITE_STATIC);

    printf("\n  id   tipo         estado  decay  contenido\n");
    printf("  %-4s %-12s %-6s  %-5s  %s\n",
           "----","------------","------","-----","--------------------");

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int    id      = sqlite3_column_int(stmt, 0);
        const char *t  = (const char *)sqlite3_column_text(stmt, 1);
        double decay   = sqlite3_column_double(stmt, 2);
        const char *c  = (const char *)sqlite3_column_text(stmt, 3);
        output_memory_row(id, t, decay, c);
        count++;
    }

    if (count == 0) printf("  (sin memorias)\n");
    printf("\n");

    sqlite3_finalize(stmt);
    return count;
}

int memory_delete(sqlite3 *db, int id) {
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM memories WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
        return 0;

    sqlite3_bind_int(stmt, 1, id);
    int ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

void memory_apply_decay(sqlite3 *db) {
    /* Cada tipo decae a distinta velocidad:
       episodic  → rápido  (×0.90 por llamada)
       semantic  → medio   (×0.97)
       procedural→ lento   (×0.995)           */
    const char *sql =
        "UPDATE memories SET decay = CASE type "
        "  WHEN 'episodic'   THEN MAX(0.0, decay * 0.90) "
        "  WHEN 'semantic'   THEN MAX(0.0, decay * 0.97) "
        "  WHEN 'procedural' THEN MAX(0.0, decay * 0.995) "
        "  ELSE decay END;";
    db_exec(db, sql);
}