# Skill: Memoria Persistente para Agentes IA

> Skill especializado para el agente de mnemOS.
> Úsalo cuando necesites decidir qué tipo de memoria crear, cuándo aplicar decay
> o cómo diseñar el flujo de recall + inject.

---

## 1. Modelo Cognitivo de mnemOS

mnemOS implementa los tres sistemas de memoria del modelo Atkinson-Shiffrin,
adaptados al contexto de agentes IA:

```
Entrada (LLM) ─┬─→ Episódica   → eventos temporales con timestamp
               ├─→ Semántica   → hechos y conceptos reutilizables
               └─→ Procedural  → hábitos, preferencias y rutinas
```

Referencia teórica: *Generative Agents: Interactive Simulacra of Human Behavior*
(Park et al., Stanford 2023) — memoria episódica, reflexión y planificación.

---

## 2. Criterios para elegir el tipo de memoria

### Episódica — `--type episodic`

Usar cuando:
- El contenido describe un **evento con fecha/hora** implícita o explícita
- La información es **específica de una sesión** y probablemente irrelevante después
- Ejemplos: bugs encontrados hoy, decisiones tomadas en esta sesión, errores cometidos

```bash
mnemos remember "Hoy resolví el bug de doble free en sqlite3_finalize" --type episodic
mnemos remember "En la sesión del 28 mar el usuario preguntó sobre punteros dobles" --type episodic
```

Decay rate: **×0.90 por ciclo** (desaparece relativamente rápido)

### Semántica — `--type semantic`

Usar cuando:
- El contenido es un **hecho objetivo** sobre el proyecto, tecnología o usuario
- La información **mejora con el uso** (el agente la necesita frecuentemente)
- Ejemplos: arquitectura del proyecto, dependencias, tecnologías usadas

```bash
mnemos remember "El proyecto usa SQLite3 con FTS5 para búsqueda de memorias" --type semantic
mnemos remember "El desarrollador trabaja en Ubuntu con gcc y Valgrind" --type semantic
```

Decay rate: **×0.97 por ciclo** (persiste mucho más)

### Procedural — `--type procedural`

Usar cuando:
- El contenido describe una **preferencia personal**, hábito o convención
- La información es casi **permanente** y caracteriza al desarrollador
- Ejemplos: estilo de código, idioma favorito, herramientas preferidas

```bash
mnemos remember "Prefiere comentarios en español en el código fuente" --type procedural
mnemos remember "Prefiere usar tabs en lugar de espacios para indentación" --type procedural
mnemos remember "Los commits deben describir el por qué, no solo el qué" --type procedural
```

Decay rate: **×0.995 por ciclo** (prácticamente permanente)

---

## 3. Escala de importance — `--importance`

El campo `importance` va de `0.0` a `1.0`:

| Valor | Cuándo usarlo |
|-------|---------------|
| `0.9 - 1.0` | Decisiones arquitecturales críticas, preferencias centrales |
| `0.6 - 0.8` | Hechos útiles frecuentes, convenciones del proyecto |
| `0.4 - 0.5` | Información de contexto general (valor por defecto) |
| `0.1 - 0.3` | Notas temporales, observaciones menores |

---

## 4. Flujo de recall → inject

El ciclo completo de uso de la memoria en el agente:

```
1. Inicio de sesión
   └─→ mnemos recall "contexto relevante" → recuperar memorias top-5

2. Durante la conversación
   ├─→ mnemos remember "nueva info" --type [tipo] → guardar aprendizajes
   └─→ mnemos log "mensaje" --session [id] → registrar diálogo

3. Al detectar preferencias del usuario
   └─→ mnemos remember "[preferencia]" --type procedural --importance 0.8

4. Fase 4 (futura): inject
   └─→ mnemos inject "prompt base" → enriquecer con memorias relevantes
```

---

## 5. El sistema de decay — cómo envejecen las memorias

El decay se aplica mediante `memory_apply_decay()` en `memory.c`:

```c
/* Cada tipo decae a distinta velocidad */
UPDATE memories SET decay = CASE type
  WHEN 'episodic'    THEN MAX(0.0, decay * 0.90)
  WHEN 'semantic'    THEN MAX(0.0, decay * 0.97)
  WHEN 'procedural'  THEN MAX(0.0, decay * 0.995)
  ELSE decay END;
```

**Regla de consolidación** (Fase 3):
- Memorias episódicas con `decay < 0.3` y `use_count > 5` → promover a semántica
- Memorias semánticas con `decay < 0.1` → candidatas para borrar
- Memorias procedurales nunca se borran automáticamente

---

## 6. El grafo de relaciones

La tabla `relations` modela conexiones entre memorias:

| Tipo | Semántica |
|------|-----------|
| `refuerza` | Una memoria confirma o complementa a otra |
| `contradice` | Una memoria invalida o contradice a otra |
| `es_parte_de` | Una memoria es un subconcepto de otra |

En Fase 3+, usar las relaciones para boosting del score de recall:
una memoria con muchos `refuerza` debe aparecer con mayor prioridad.

---

## 7. Escenarios de uso del agente OpenCode

| El usuario dice | El agente hace |
|----------------|----------------|
| "Recuerda que prefiero X" | `mnemos remember "..." --type procedural` |
| "¿Qué sabes de este proyecto?" | `mnemos recall "proyecto"` + resumir |
| "¿Recuerdas lo de ayer?" | `mnemos recall "[tema]"` filtrando por fecha |
| "Muestra mis preferencias" | `mnemos list --type procedural` |
| "Olvida eso" | `mnemos forget [id]` tras confirmar con el usuario |
