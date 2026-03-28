#ifndef MEMORY_H
#define MEMORY_H

#include <sqlite3.h>
#include <time.h>

typedef struct {
    int id;
    char content[1024];
    char type[16];
    double importance;
    double decay;
    char create_at[32];
    int use_count;
} Memory;

int memory_insert(sqlite3 *db, const char *content, const char *type, double importance);
int memory_list(sqlite3 *db, const char *type);
int memory_delete(sqlite3 *db, int id);
void memory_apply_decay(sqlite3 *db);

#endif