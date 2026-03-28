#include <stdio.h>
#include <string.h>
#include "output.h"

void output_separator(int n) {
    for (int i = 0; i < n; i++) printf("-");
    printf("\n");
}

void output_header(const char *cols[], int n) {
    for (int i = 0; i < n; i++)
        printf("%-20s", cols[i]);
    printf("\n");
    output_separator(n * 20);
}

void output_row(const char *vals[], int n) {
    for (int i = 0; i < n; i++)
        printf("%-20s", vals[i] ? vals[i] : "NULL");
    printf("\n");
}

void output_memory_row(int id, const char *type,
                       double decay, const char *content) {
    /* Indicador visual de decay */
    const char *salud;
    if      (decay > 0.7) salud = "●●●";
    else if (decay > 0.4) salud = "●●○";
    else                  salud = "●○○";

    printf("[%3d] %-10s %s  %.2f  %s\n",
           id, type, salud, decay, content);
}