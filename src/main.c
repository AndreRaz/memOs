#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "commands.h"


int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Cmem -- memory for AI agents\n");
        printf("Uso: Cmem <comandos> [opciones]\n\n");
        printf("Comandos disponibles:\n");
        printf("  remember  Guarda una nueva memoria\n");
        printf("  log       Registra un mensaje en una sesión\n");
        printf("  list      Lista memorias\n");
        printf("  recall    Busca memorias relevantes\n");
        printf("  forget    Elimina una memoria por id\n");
        printf("  stats     Muestra estadísticas\n");
        return 0;
    }


    const char *db_path = getenv("MNEMOS_DB");
    if (!db_path) db_path = "data/mnemos.db";

    sqlite3 *db = db_open(db_path);
    db_init_schema(db);

    if      (strcmp(argv[1], "remember") == 0) mem_remember(db, argc-2, argv+2);
    else if (strcmp(argv[1], "log")      == 0) mem_log(db, argc-2, argv+2);
    else if (strcmp(argv[1], "list")     == 0) mem_list(db, argc-2, argv+2);
    else if (strcmp(argv[1], "recall")   == 0) mem_recall(db, argc-2, argv+2);
    else if (strcmp(argv[1], "forget")   == 0) mem_forget(db, argc-2, argv+2);
    else if (strcmp(argv[1], "stats")    == 0) mem_stats(db);
    else fprintf(stderr, "Comando desconocido: %s\n", argv[1]);

    db_close(db);
    return 0;
}