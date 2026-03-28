# Skill: Buenas Prácticas en C — mnemOS

> Skill especializado para el agente de mnemOS.
> Úsalo antes de escribir, modificar o revisar cualquier código C del proyecto.

---

## 1. Convenciones del proyecto

### Nomenclatura

```c
/* Variables y funciones: snake_case */
int memory_count;
void mem_remember(sqlite3 *db, int argc, char *argv[]);

/* Constantes: SNAKE_CASE_MAYÚSCULAS */
#define MAX_CONTENT_LEN 1024

/* Structs: PascalCase */
typedef struct { ... } MemoryEntry;
```

### Comentarios

- Siempre en **español**
- Los bloques de función llevan un comentario de propósito:

```c
/* Inserta una nueva entrada en la tabla memories.
   Devuelve el id asignado, o -1 si falla. */
int memory_insert(sqlite3 *db, const char *content,
                  const char *type, double importance) {
```

- Comentarios inline: solo para código no obvio, en la misma línea

```c
sqlite3_bind_text(stmt, 1, content, -1, SQLITE_STATIC);  /* no copiar el string */
```

---

## 2. Seguridad en buffers — reglas obligatorias

### ❌ Nunca usar `sprintf`

```c
/* MAL — buffer overflow si content es largo */
char sql[256];
sprintf(sql, "INSERT INTO memories VALUES ('%s')", content);
```

### ✅ Siempre usar `snprintf`

```c
/* BIEN — trunca de forma segura */
char sql[256];
snprintf(sql, sizeof(sql),
    "SELECT id FROM memories WHERE type = '%s' LIMIT 5;", type);
```

### ✅ Para queries con datos del usuario, SIEMPRE usar bind

```c
/* MEJOR — nunca interpolar datos del usuario en SQL */
sqlite3_prepare_v2(db, "INSERT INTO ... VALUES (?)", -1, &stmt, NULL);
sqlite3_bind_text(stmt, 1, contenido_usuario, -1, SQLITE_STATIC);
```

---

## 3. Gestión de memoria — checklist

Antes de dar por terminado cualquier bloque de código:

- [ ] Cada `malloc` / `calloc` tiene su `free` correspondiente
- [ ] Cada `sqlite3_prepare_v2` tiene su `sqlite3_finalize`
- [ ] Cada `sqlite3_open` tiene su `sqlite3_close`
- [ ] Los punteros se validan antes de derreferenciar: `if (!ptr) return;`
- [ ] Los strings de Valgrind no quedan sin liberar (`sqlite3_free(err)`)

### Patrón de liberación segura

```c
char *err = NULL;
if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
    fprintf(stderr, "Error SQL: %s\n", err);
    sqlite3_free(err);   /* OBLIGATORIO — liberar el mensaje de error */
    return 0;
}
```

---

## 4. Cómo añadir un módulo nuevo

Si el agente necesita crear un módulo nuevo (ej: `src/embed.c`):

1. **Crear el header** `include/embed.h`:
```c
#ifndef EMBED_H
#define EMBED_H
#include <sqlite3.h>
/* Declaraciones de funciones públicas */
void embed_compute(sqlite3 *db, int memory_id);
#endif
```

2. **Crear el fuente** `src/embed.c`:
```c
#include <stdio.h>
#include "embed.h"
/* Implementación */
```

3. **Añadir al Makefile** (la variable `SRC`):
```makefile
SRC = src/main.c src/db.c src/memory.c src/commands.c src/output.c src/embed.c
```

4. **Compilar y verificar**: `make clean && make`

5. **Añadir tests**: al menos un caso en `test/test.sh`

---

## 5. Workflow de Valgrind — interpretar la salida

Ejecutar: `make valgrind`

### Tipos de leak (ordenados por gravedad)

| Tipo | Gravedad | Acción |
|------|----------|--------|
| `definitely lost` | 🔴 Crítico | Buscar y añadir el `free` faltante |
| `indirectly lost` | 🔴 Crítico | Resolver el `definitely lost` asociado |
| `possibly lost` | 🟡 Revisar | Generalmente punteros con aritmética |
| `still reachable` | 🟢 Aceptable | SQLite internal, común en finalización |

### Meta del proyecto: 0 `definitely lost` en todos los comandos

### Ejemplo de output problemático

```
LEAK SUMMARY:
   definitely lost: 24 bytes in 1 blocks
```

→ Buscar el `malloc` sin `free` con `--track-origins=yes` (ya incluido en `make valgrind`).

---

## 6. Estilo visual del output en terminal

El proyecto usa indicadores visuales definidos en `output.c`:

```c
/*  ●●● → decay > 0.7  (memoria fresca)   */
/*  ●●○ → decay > 0.4  (memoria envejecida) */
/*  ●○○ → decay ≤ 0.4  (memoria próxima a expirar) */
```

Para añadir output nuevo, usar SIEMPRE `output_memory_row()` u `output_header()` 
en lugar de `printf` directos, para mantener consistencia visual.

---

## 7. Headers — guard obligatorio

Todo `.h` del proyecto **debe** tener include guard:

```c
#ifndef NOMBRE_H
#define NOMBRE_H

/* contenido */

#endif
```

Nunca usar `#pragma once` — preferimos la forma estándar ANSI C.
