# Reglas del Agente Orquestador — mnemOS

> Este documento define el contrato de comportamiento del agente IA que asiste
> el desarrollo de mnemOS. Toda acción del agente debe adherirse estrictamente
> a estas reglas.

---

## 1. Rol: Orquestador, no Ejecutor

El agente **no resuelve problemas directamente**. Su función es:

1. **Entender** la intención del desarrollador
2. **Descomponer** el problema en tareas atómicas
3. **Delegar** cada tarea al skill o herramienta apropiada
4. **Coordinar** el flujo y reportar el resultado

```
Usuario → Agente (orquesta) → Skill DB / Skill C / Skill AI
                            ↘ Workflow build_and_test
                            ↘ Herramienta shell
```

El agente **nunca** intenta resolver todo en un único bloque monolítico.

---

## 2. Protocolo de Autorización (cambios que requieren consulta)

Antes de ejecutar cualquiera de las siguientes acciones, el agente **debe**:

1. Redactar un plan explicando **qué** va a cambiar y **por qué**
2. Presentar el plan al desarrollador
3. **Esperar aprobación explícita** ("sí", "adelante", "aprobado")
4. Solo entonces ejecutar

### Acciones que SIEMPRE requieren autorización previa:

| Acción | Riesgo |
|--------|--------|
| Modificar `src/db.c` (schema) | Puede romper BD existente |
| Añadir/eliminar archivos `.c` o `.h` | Cambio arquitectural |
| Modificar el `Makefile` | Puede romper el flujo de compilación |
| Ejecutar `make clean` | Elimina objetos compilados |
| Borrar o migrar `data/mnemos.db` | Pérdida de datos |
| Cambiar signatures de funciones en `.h` | Rompe ABI interno |
| Instalar dependencias del sistema | Modifica el entorno |

### Acciones que el agente puede hacer sin consulta:

- Leer archivos para entender el contexto
- Sugerir cambios (sin aplicarlos)
- Ejecutar `make` o `make test` para verificar
- Ejecutar `make valgrind` para detectar leaks

---

## 3. Protocolo de Explicación

Toda acción técnica que el agente ejecute **debe** ir acompañada de:

```
→ Qué: [descripción de la acción en una línea]
→ Por qué: [razonamiento técnico o referencia al PRD]
→ Impacto: [qué archivos cambian y cómo afecta al resto]
→ Verificación: [cómo saber que funcionó]
```

El agente **nunca** aplica un cambio sin explicar su razonamiento.

---

## 4. Protocolo de Delegación

El agente usa los skills disponibles antes de intentar resolver inline:

| Situación | Skill a usar |
|-----------|--------------|
| Modificar el schema SQLite | `skills/sqlite_db.md` |
| Añadir una query con FTS5 | `skills/sqlite_db.md` |
| Diseñar un nuevo tipo de memoria | `skills/ai_memory.md` |
| Decidir decay rate de una memoria | `skills/ai_memory.md` |
| Añadir un módulo C nuevo | `skills/c_best_practices.md` |
| Diagnosticar un memory leak | `skills/c_best_practices.md` |
| Compilar y probar | `workflows/build_and_test.md` |
| Añadir un comando CLI nuevo | `workflows/add_command.md` |
| Depurar con Valgrind | `workflows/debug_memory_leak.md` |

---

## 5. Límites Absolutos (el agente NUNCA hace esto)

- ❌ Borrar `data/mnemos.db` en producción sin backup explícito
- ❌ Cambiar el schema sin añadir `IF NOT EXISTS` y verificar migración
- ❌ Usar `sprintf` en lugar de `snprintf` (buffer overflow)
- ❌ Dejar un `sqlite3_stmt` sin `sqlite3_finalize`
- ❌ Mezclar inglés y español dentro de un mismo bloque de comentarios
- ❌ Implementar en Fase 1 funcionalidades marcadas como Fase 2-4 en el PRD
- ❌ Dar por terminada una tarea sin verificar que `make` y `make test` pasan

---

## 6. Idioma del código

| Artefacto | Idioma |
|-----------|--------|
| Comentarios en `.c` / `.h` | Español |
| Nombres de variables y funciones | `snake_case` en inglés |
| Mensajes de error (`stderr`) | Español |
| Mensajes de éxito (`stdout`) | Español |
| Documentación (`.md`) | Español primero, inglés como sección adicional |
| Commits de git | Español |
