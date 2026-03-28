# Skill: Manejo de Base de Datos SQLite3

> Skill especializado para el agente de mnemOS.
> Úsalo cuando el agente necesite interactuar con el schema, queries o FTS5.

---

## 1. Estructura del Schema

El schema completo vive en `src/db.c` como string estático `SCHEMA`. Toda
modificación del schema debe respetar estas reglas:

- Siempre usar `CREATE TABLE IF NOT EXISTS` — nunca `CREATE TABLE`
- Siempre usar `CREATE TRIGGER IF NOT EXISTS`
- Nunca eliminar columnas (SQLite no lo soporta sin recrear la tabla)
- Para añadir columnas: `ALTER TABLE nombre ADD COLUMN ...`

### Tablas del proyecto

```
memories       → memorias individuales con decay
memories_fts   → tabla virtual FTS5 sincronizada mediante triggers
relations      → grafo de relaciones entre memorias
sessions       → sesiones de conversación
messages       → log de mensajes por sesión
```

---

## 2. Patrón seguro de query con parámetros

**SIEMPRE** usar este patrón. Nunca concatenar strings con datos del usuario.

```c
sqlite3_stmt *stmt;
const char *sql = "SELECT id, content FROM memories WHERE type = ?;";

if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    fprintf(stderr, "Error preparando query: %s\n", sqlite3_errmsg(db));
    return -1;   /* propagar el error, no continuar */
}

sqlite3_bind_text(stmt, 1, tipo, -1, SQLITE_STATIC);

while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id          = sqlite3_column_int(stmt, 0);
    const char *txt = (const char *)sqlite3_column_text(stmt, 1);
    /* procesar fila */
}

sqlite3_finalize(stmt);   /* OBLIGATORIO — siempre finalizar */
```

### Reglas de binding

| Tipo C | Función SQLite |
|--------|----------------|
| `const char *` | `sqlite3_bind_text(stmt, N, val, -1, SQLITE_STATIC)` |
| `int` | `sqlite3_bind_int(stmt, N, val)` |
| `double` | `sqlite3_bind_double(stmt, N, val)` |
| `NULL` | `sqlite3_bind_null(stmt, N)` |

---

## 3. FTS5 — Búsqueda de texto completo

La tabla `memories_fts` es una tabla virtual FTS5 sincronizada automáticamente
con `memories` mediante dos triggers (`memories_ai` para INSERT, `memories_ad`
para DELETE).

### Query de recall (búsqueda)

```c
const char *sql =
    "SELECT m.id, m.type, m.decay, m.content "
    "FROM memories m "
    "JOIN memories_fts f ON m.id = f.rowid "
    "WHERE memories_fts MATCH ? "
    "ORDER BY m.decay DESC "
    "LIMIT 5;";
```

### Diagnóstico de FTS5

Si FTS5 da resultados incorrectos o errores de integridad:

```sql
-- Verificar integridad
INSERT INTO memories_fts(memories_fts) VALUES ('integrity-check');

-- Reconstruir índice completo
INSERT INTO memories_fts(memories_fts) VALUES ('rebuild');
```

### Sintaxis de búsqueda FTS5

| Expresión | Significado |
|-----------|-------------|
| `"sqlite"` | Búsqueda exacta de token |
| `"sqlite AND decay"` | Ambos tokens presentes |
| `"sqlite OR decay"` | Cualquiera de los dos |
| `"sql*"` | Prefijo (búsqueda por prefijo) |

---

## 4. Cómo añadir una tabla nueva al schema

1. Añadir la definición en la cadena `SCHEMA` de `src/db.c`
2. Usar `IF NOT EXISTS` para no romper instalaciones existentes
3. Si la tabla tiene relaciones con `memories`, añadir `ON DELETE CASCADE`
4. Compilar: `make` → verificar que compila sin errores
5. Probar manualmente: `./mnemos stats` con una DB nueva

**Si la tabla ya existe en producción y necesita una columna nueva:**

```sql
ALTER TABLE nombre ADD COLUMN nueva_col TEXT DEFAULT '';
```

Esto se puede ejecutar con `db_exec(db, sql)` de forma segura.

---

## 5. Errores comunes y soluciones

| Error | Causa probable | Solución |
|-------|----------------|----------|
| `no such table: memories_fts` | FTS5 no disponible o DB vieja | Recompilar SQLite con FTS5 o recrear la DB |
| `FOREIGN KEY constraint failed` | `foreign_keys` no está ON | Verificar que `PRAGMA foreign_keys = ON` se ejecuta en `db_open` |
| `attempt to write a readonly database` | Permisos del archivo | `chmod 644 data/mnemos.db` |
| `database is locked` | Dos procesos accediendo | Solo un proceso a la vez; usar `WAL` mode si necesitas concurrencia |
| Resultados FTS5 vacíos | Trigger no se ejecutó correctamente | Reconstruir: `INSERT INTO memories_fts(memories_fts) VALUES ('rebuild')` |
