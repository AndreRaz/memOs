#ifndef COMMANDS_H
#define COMMANDS_H

#include <sqlite3.h>

void mem_remember(sqlite3 *db, int argc, char *argv[]);
void mem_log(sqlite3 *db, int argc, char *argv[]);
void mem_list(sqlite3 *db, int argc, char *argv[]);
void mem_recall(sqlite3 *db, int argc, char *argv[]);
void mem_forget(sqlite3 *db, int argc, char *argv[]);
void mem_stats(sqlite3 *db);

#endif

