#include <stdio.h>
#include <stdlib.h>
#include "db.h"

/* Schema completo embebido directamente en el código */
static const char *SCHEMA =
    "CREATE TABLE IF NOT EXISTS memories ("
    "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  content    TEXT    NOT NULL,"
    "  type       TEXT    NOT NULL CHECK(type IN ('episodic','semantic','procedural')),"
    "  importance REAL    NOT NULL DEFAULT 0.5,"
    "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "  last_used  DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "  use_count  INTEGER DEFAULT 0,"
    "  decay      REAL    DEFAULT 1.0"
    ");"
    "CREATE VIRTUAL TABLE IF NOT EXISTS memories_fts"
    "  USING fts5(content, content='memories', content_rowid='id');"
    "CREATE TRIGGER IF NOT EXISTS memories_ai"
    "  AFTER INSERT ON memories BEGIN"
    "    INSERT INTO memories_fts(rowid, content) VALUES (new.id, new.content);"
    "  END;"
    "CREATE TRIGGER IF NOT EXISTS memories_ad"
    "  AFTER DELETE ON memories BEGIN"
    "    INSERT INTO memories_fts(memories_fts, rowid, content)"
    "    VALUES ('delete', old.id, old.content);"
    "  END;"
    "CREATE TABLE IF NOT EXISTS sessions ("
    "  id         TEXT PRIMARY KEY,"
    "  summary    TEXT,"
    "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");"
    "CREATE TABLE IF NOT EXISTS messages ("
    "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  session_id TEXT NOT NULL REFERENCES sessions(id) ON DELETE CASCADE,"
    "  role       TEXT NOT NULL CHECK(role IN ('user','assistant')),"
    "  content    TEXT NOT NULL,"
    "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");"
    "CREATE TABLE IF NOT EXISTS relations ("
    "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  from_id    INTEGER NOT NULL REFERENCES memories(id) ON DELETE CASCADE,"
    "  to_id      INTEGER NOT NULL REFERENCES memories(id) ON DELETE CASCADE,"
    "  type       TEXT NOT NULL CHECK(type IN ('refuerza','contradice','es_parte_de')),"
    "  weight     REAL NOT NULL DEFAULT 1.0,"
    "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
    ");";

sqlite3 *db_open(const char *path) {
    sqlite3 *db;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        fprintf(stderr, "Error abriendo BD: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    /* Activar foreign keys */
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    return db;
}

void db_close(sqlite3 *db) {
    sqlite3_close(db);
}

int db_exec(sqlite3 *db, const char *sql) {
    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Error SQL: %s\n", err);
        sqlite3_free(err);
        return 0;
    }
    return 1;
}

void db_init_schema(sqlite3 *db) {
    char *err = NULL;
    if (sqlite3_exec(db, SCHEMA, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Error creando schema: %s\n", err);
        sqlite3_free(err);
        exit(1);
    }
}