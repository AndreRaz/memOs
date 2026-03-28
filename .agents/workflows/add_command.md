---
description: añadir un nuevo comando CLI a mnemOS
---

# Workflow: Añadir un Comando CLI

Pasos exactos para extender mnemOS con un nuevo comando CLI. Seguir este
orden evita referencias sin definir y errores de linkado.

## Ejemplo: añadir el comando `decay`

### Paso 1: declarar la función en `include/commands.h`

```c
void mem_decay(sqlite3 *db, int argc, char *argv[]);
```

Regla: el prefijo es siempre `mem_` para los comandos CLI.

### Paso 2: implementar en `src/commands.c`

```c
/* ── decay ───────────────────────────────────────────────
   Uso: mnemos decay
   Aplica decay diferencial a todas las memorias.
*/
void mem_decay(sqlite3 *db, int argc, char *argv[]) {
    (void)argc; (void)argv;   /* parámetros no usados — silenciar warning */
    memory_apply_decay(db);
    printf("✓ Decay aplicado a todas las memorias.\n");
}
```

Notas importantes:
- Si el comando no usa `argc`/`argv`, silenciarlos con `(void)` para evitar warning `-Wunused`
- Comentario de bloque en español, descripción de uso incluida
- Errores a `stderr`, confirmaciones a `stdout`

### Paso 3: registrar en `src/main.c`

Añadir el nuevo `else if` en el bloque de despacho:

```c
else if (strcmp(argv[1], "decay")  == 0) mem_decay(db, argc-2, argv+2);
```

Mantener la alineación visual de los `== 0)` para legibilidad.

### Paso 4: actualizar el menú de ayuda en `main.c`

```c
printf("  decay     Aplica decay diferencial a todas las memorias\n");
```

### Paso 5: añadir test en `test/test.sh`

```bash
echo ""
echo -e "${CYAN}─── decay ───────────────────────────────────${RESET}"

assert_output_contains \
    "decay ejecuta sin error" \
    "Decay aplicado" \
    run decay
```

### Paso 6: compilar y testear

```bash
make clean && make && make test
```

Resultado esperado: el nuevo test aparece con `✓ PASS`.

### Paso 7: documentar en README.md

Añadir el nuevo comando a la tabla de comandos con su descripción y ejemplo.

---

## Checklist completo

- [ ] Declarado en `include/commands.h`
- [ ] Implementado en `src/commands.c` (con comentario de bloque)
- [ ] Registrado en `src/main.c` (bloque de despacho + menú de ayuda)
- [ ] Test añadido en `test/test.sh`
- [ ] `make clean && make && make test` → todos PASS
- [ ] README actualizado
