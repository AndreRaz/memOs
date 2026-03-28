<div align="center">

# mnemOS

**Motor de Memoria Persistente para Agentes IA**

*Memoria episódica · semántica · procedural — con decay diferencial — en C puro*

[![Lenguaje](https://img.shields.io/badge/lenguaje-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Base de datos](https://img.shields.io/badge/BD-SQLite3%20%2B%20FTS5-green.svg)](https://www.sqlite.org/)
[![Estado](https://img.shields.io/badge/fase-1%20en%20desarrollo-orange.svg)]()
[![Licencia](https://img.shields.io/badge/licencia-MIT-lightgrey.svg)]()

</div>

---

## ¿Qué es mnemOS?

mnemOS es un **motor de memoria persistente** para agentes IA, construido desde
cero en C con SQLite3. Su propósito es dotar a cualquier LLM de tres tipos de
memoria inspirados en neurociencia cognitiva: **episódica**, **semántica** y
**procedural**.

A diferencia de soluciones como MemGPT (Python + Pinecone), mnemOS es un motor
de sistemas puro: sin frameworks, sin bases de datos vectoriales externas, sin
runtime de Python. Esto lo convierte en una pieza de infraestructura **ligera,
auditable y portable**.

```
Tú (lenguaje natural)
    │
    ▼
Agente OpenCode (mnemos.md)
    │
    ▼
CLI mnemos  ──→  Motor C  ──→  SQLite3
                                  ├── memories (decay diferencial)
                                  ├── memories_fts (FTS5)
                                  ├── relations (grafo)
                                  ├── sessions
                                  └── messages
```

---

## Inspiración técnica

- **Generative Agents** (Park et al., Stanford 2023) — memoria episódica, reflexión y planificación en agentes
- **MemGPT** — arquitectura de contexto virtual para LLMs con memoria paginada
- **Modelo Atkinson-Shiffrin** — memoria sensorial, de corto y largo plazo

---

## Arquitectura

### Capa 1 — Almacenamiento (SQLite3 + FTS5)

| Tabla | Descripción |
|-------|-------------|
| `memories` | Memorias individuales con `content`, `type`, `importance`, `decay` |
| `memories_fts` | Tabla virtual FTS5 sincronizada con triggers para búsqueda full-text |
| `relations` | Grafo de relaciones entre memorias (`refuerza`, `contradice`, `es_parte_de`) |
| `sessions` | Sesiones de conversación identificadas por ID único |
| `messages` | Log completo de mensajes por sesión con `role` (user/assistant) |

### Capa 2 — Motor C

| Módulo | Responsabilidad |
|--------|----------------|
| `db.c` | Conexión, schema y ejecución de queries |
| `memory.c` | Inserción, listado, eliminación y decay diferencial |
| `commands.c` | Implementación de cada comando CLI |
| `output.c` | Renderizado de tablas y resultados en terminal |

### Capa 3 — Integración (Agente OpenCode)

Agente personalizado en `~/.config/opencode/commands/mnemos.md` que:
- Interpreta lenguaje natural del desarrollador
- Traduce intenciones a comandos `mnemos`
- Devuelve resultados con contexto enriquecido

---

## Tipos de Memoria

| Tipo | Descripción | Ejemplo | Decay por ciclo |
|------|-------------|---------|-----------------|
| **Episódica** | Eventos con marca de tiempo | "Hoy resolví el bug de FTS5" | ×0.90 (rápido) |
| **Semántica** | Hechos y conceptos reutilizables | "El proyecto usa SQLite3" | ×0.97 (medio) |
| **Procedural** | Hábitos y preferencias | "Prefiere tabs sobre espacios" | ×0.995 (lento) |

Las memorias **envejecen y se consolidan** igual que en el cerebro humano.

---

## Instalación

### Dependencias (Ubuntu/Debian)

```bash
sudo apt install -y libsqlite3-dev sqlite3 valgrind
```

### Compilar

```bash
git clone https://github.com/tu-usuario/mnemos.git
cd mnemos
make
```

### Verificar

```bash
make test       # suite de tests shell
make valgrind   # verificar que no hay memory leaks
```

---

## Uso

### Guardar una memoria

```bash
# Memoria semántica (default)
./mnemos remember "El proyecto usa SQLite3 con FTS5 para búsqueda"

# Memoria procedural con importancia alta
./mnemos remember "Prefiero comentarios en español" --type procedural --importance 0.9

# Memoria episódica
./mnemos remember "Hoy resolví el bug de los triggers FTS5" --type episodic
```

### Buscar memorias relevantes

```bash
./mnemos recall "SQLite"
./mnemos recall "preferencias"
```

### Listar memorias

```bash
./mnemos list                        # todas
./mnemos list --type procedural      # solo procedurales
./mnemos list --type episodic        # solo episódicas
```

### Registrar una sesión de conversación

```bash
./mnemos log "Revisando el módulo de memoria" --session sesion-01
./mnemos log "El bug era un doble free" --session sesion-01 --role assistant
```

### Eliminar una memoria

```bash
./mnemos forget 3   # elimina la memoria con id=3
```

### Ver estadísticas

```bash
./mnemos stats
```

**Salida de ejemplo:**

```
═══════════════════════════════
  mnemOS — estadísticas
═══════════════════════════════

  tipo          total   decay     import.
  ------------  ------  --------  --------
  episodic      3       0.85      0.60
  procedural    5       0.99      0.82
  semantic      8       0.91      0.70

  Sesiones registradas: 2
  Mensajes en log:      7
```

---

## Roadmap

| Fase | Nombre | Alcance | Estado |
|------|--------|---------|--------|
| **F1** | Core CLI + SQLite | remember, log, list, recall, forget, stats | ✅ En desarrollo |
| **F2** | Búsqueda semántica | Embeddings vía API, similitud coseno en C | ⏳ Planeada |
| **F3** | Daemon consolidador | fork() background, decay automático, merge | ⏳ Planeada |
| **F4** | Integración LLM | libcurl + API, inject, agente OpenCode completo | ⏳ Planeada |

### Comandos futuros (F2-F4)

```bash
mnemos decay                    # aplica decay manualmente
mnemos graph                    # visualiza el grafo de relaciones
mnemos inject "prompt"          # enriquece un prompt con memorias relevantes
mnemos daemon start|stop|status # gestiona el consolidador en background
mnemos export --format json|csv # exporta la base de datos
```

---

## Comparativa con MemGPT

| Característica | mnemOS | MemGPT |
|----------------|--------|--------|
| Lenguaje | C puro | Python |
| Almacenamiento | SQLite3 embebido | Pinecone (externo) |
| Dependencias | 1 (libsqlite3) | Docenas de paquetes |
| Memoria |  < 5 MB | > 200 MB (runtime Python) |
| Portabilidad | Binario único | Entorno Python requerido |
| Tipos de memoria | Episódica, semántica, procedural | Memoria de trabajo + archival |
| Decay diferencial | ✅ | ❌ |

---

## Estructura del proyecto

```
mnemos/
├── src/
│   ├── main.c        # punto de entrada y despacho de comandos
│   ├── db.c          # capa de base de datos y schema
│   ├── memory.c      # lógica de memorias y decay
│   ├── commands.c    # implementación de comandos CLI
│   └── output.c      # renderizado de resultados
├── include/
│   ├── db.h
│   ├── memory.h
│   ├── commands.h
│   └── output.h
├── test/
│   └── test.sh       # suite de tests shell
├── data/
│   └── mnemos.db     # base de datos SQLite (generada al compilar)
├── .agents/
│   ├── rules/        # reglas del agente orquestador
│   ├── skills/       # skills especializados (SQLite, IA, C)
│   └── workflows/    # flujos de trabajo documentados
└── Makefile
```

---

## Métricas de éxito (Fase 1)

- ✅ Zero memory leaks reportados por Valgrind
- ✅ Tiempo de respuesta < 50ms para recall con FTS5
- ✅ Schema migra con `IF NOT EXISTS` sin pérdida de datos
- ✅ Todos los comandos testeados con suite shell

---

---

## English Summary

**mnemOS** is a persistent memory engine for AI agents, built in pure C with
SQLite3. It gives any LLM three types of cognitive memory — episodic, semantic
and procedural — with differential decay rates inspired by the Atkinson-Shiffrin
model and *Generative Agents* (Stanford, 2023).

### Key features
- Pure C — no Python runtime, no external vector DB
- SQLite3 + FTS5 for full-text search
- Differential memory decay (episodic decays fast, procedural almost never)
- OpenCode custom agent for natural language interaction
- Designed as a portfolio differentiator: systems + AI + Spanish documentation

### Quick start
```bash
sudo apt install -y libsqlite3-dev
make && make test
./mnemos remember "Your first memory" --type semantic
./mnemos stats
```

---

<div align="center">

Hecho con ♥ en C — porque a veces la mejor abstracción es ninguna abstracción

*"La memoria no es lo que recordamos, sino lo que nos permite olvidar todo lo demás"*

</div>
