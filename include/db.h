#ifndef DB_H
#define DB_H
#include <sqlite3.h>


sqlite3 *db_open(const char *path);
void db_close(sqlite3 *db);
int db_exec(sqlite3 *db, const char *sql);
void db_init_schema(sqlite3 *db);

#endif


