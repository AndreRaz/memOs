---
description: compilar, limpiar y ejecutar los tests de mnemOS
---

# Workflow: Build & Test

Pasos para compilar el proyecto desde cero, ejecutar la suite de tests y
verificar que no hay memory leaks.

## Paso 1: limpiar artefactos anteriores

```bash
make clean
```

Elimina todos los `.o`, el binario `mnemos` y la base de datos de producción.

> ⚠️ Si quieres conservar `data/mnemos.db`, comenta la línea del `Makefile`
> que lo borra, o haz un backup primero.

## Paso 2: compilar

```bash
make
```

Resultado esperado: ningún `error:` ni `warning:` de gcc.
Si hay warnings, tratar como errores — el agente NO dará la tarea por
terminada con warnings pendientes.

## Paso 3: ejecutar tests

```bash
make test
```

Resultado esperado: todos los tests con `✓ PASS`. Si alguno falla:
1. Leer el output del test fallido
2. Ver qué comando CLI falló
3. Ir al archivo correspondiente en `src/commands.c`
4. Corregir y volver al Paso 2

## Paso 4: verificar leaks (opcional pero recomendado)

```bash
make valgrind
```

Meta: `0 bytes in 0 blocks` en el campo `definitely lost`.
Ver skill `c_best_practices.md` §5 para interpretar el output.

## Resumen del flujo completo

```
make clean → make → make test → make valgrind
```
