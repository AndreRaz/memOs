---
description: diagnosticar y eliminar memory leaks con Valgrind
---

# Workflow: Depuración de Memory Leaks

Pasos para identificar y corregir fugas de memoria en mnemOS usando Valgrind.

## Paso 1: ejecutar Valgrind con el comando sospechoso

```bash
make valgrind
```

Esto ejecuta: `valgrind --leak-check=full --track-origins=yes ./mnemos stats`

Para probar un comando específico diferente:

```bash
valgrind --leak-check=full --track-origins=yes \
         --show-leak-kinds=all \
         ./mnemos remember "texto de prueba" --type semantic
```

## Paso 2: interpretar el output

### Output limpio (objetivo del proyecto)

```
LEAK SUMMARY:
   definitely lost: 0 bytes in 0 blocks
   indirectly lost: 0 bytes in 0 blocks
     possibly lost: 0 bytes in 0 blocks
   still reachable: N bytes in N blocks   ← SQLite internals, aceptable
```

### Output con problema

```
LEAK SUMMARY:
   definitely lost: 48 bytes in 2 blocks  ← PROBLEMA — hay que resolverlo
```

### Tipos de leak

| Tipo | Gravedad | Descripción |
|------|----------|-------------|
| `definitely lost` | 🔴 Crítico | Puntero perdido, nunca liberado. Buscar y añadir `free` |
| `indirectly lost` | 🔴 Crítico | Causado por el `definitely lost` anterior. Se resuelve junto |
| `possibly lost` | 🟡 Revisar | Puntero con aritmética. Revisar si es intencional |
| `still reachable` | 🟢 Aceptable | Memoria accesible al cierre. SQLite lo hace así en clean shutdown |

## Paso 3: localizar la línea del leak

Leer el bloque de stack trace que Valgrind imprime:

```
48 bytes in 2 blocks are definitely lost in loss record 1 of 2
   at 0x...: malloc (vg_replace_malloc.c:...)
   at 0x...: sqlite3_exec (sqlite3.c:...)
   at 0x...: db_init_schema (db.c:72)    ← AQUÍ está el origen
   at 0x...: main (main.c:26)
```

→ El leak viene de `db_init_schema`. Revisar si la variable `err` fue
liberada con `sqlite3_free(err)` en todos los caminos.

## Paso 4: corregir

### Caso más común — error sin liberar mensaje de Valgrind

```c
/* MAL */
char *err = NULL;
sqlite3_exec(db, sql, NULL, NULL, &err);
if (err) fprintf(stderr, "%s\n", err);
/* ← falta sqlite3_free(err) */

/* BIEN */
char *err = NULL;
if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
    fprintf(stderr, "Error SQL: %s\n", err);
    sqlite3_free(err);   /* siempre liberar */
}
```

### Caso común — stmt sin finalizar

```c
/* MAL — si hay return anticipado antes de finalize */
sqlite3_stmt *stmt;
sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
if (condicion) return;   /* ← stmt quedó abierto */
sqlite3_finalize(stmt);

/* BIEN — finalizar antes de cualquier return */
sqlite3_stmt *stmt;
sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
if (condicion) {
    sqlite3_finalize(stmt);
    return;
}
sqlite3_finalize(stmt);
```

## Paso 5: verificar la corrección

```bash
make clean && make && make valgrind
```

Resultado esperado: `definitely lost: 0 bytes in 0 blocks`

---

## Referencia rápida de funciones de liberación en este proyecto

| Recurso abierto con... | Siempre cerrar con... |
|------------------------|-----------------------|
| `sqlite3_open()` | `sqlite3_close()` |
| `sqlite3_prepare_v2()` | `sqlite3_finalize()` |
| `sqlite3_exec(..., &err)` y err != NULL | `sqlite3_free(err)` |
| `malloc()` / `calloc()` | `free()` |
