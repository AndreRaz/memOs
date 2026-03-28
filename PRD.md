mnemOS
Memory Operating System for AI Agents
Agente personalizado para OpenCode
Proyecto
mnemOS — memory OS for AI Agents
Tipo
Agente OpenCode personalizado + CLI en C
Stack
C · SQLite3 · libcurl · cJSON · OpenCode
Autor
Portafolio personal — C & IA
Versión PRD
1.0.0  —  Marzo 2026
Estado
En desarrollo activo — Fase 1




1. Visión general
mnemOS es un motor de memoria persistente para agentes IA, construido desde cero en C con SQLite3. Su propósito es dotar a cualquier LLM de tres tipos de memoria inspirados en neurociencia cognitiva: episódica, semántica y procedural.
A diferencia de soluciones existentes como MemGPT (Python + Pinecone), mnemOS es un motor de sistemas puro: sin frameworks, sin bases de datos vectoriales externas, sin runtime de Python. Esto lo convierte en una pieza de infraestructura ligera, auditable y portable.
El agente de OpenCode actúa como la interfaz conversacional de mnemOS: el desarrollador habla con él en lenguaje natural, y el agente traduce esas intenciones a comandos del motor.

1.1 Propuesta de valor
Motor de memoria en C puro — sin dependencias de alto nivel, máximo control sobre rendimiento y comportamiento.
Tres capas de memoria con decay diferencial — las memorias envejecen y se consolidan igual que en el cerebro humano.
Agnóstico al modelo — funciona con Claude, GPT-4, Llama, Mistral o cualquier LLM.
Integración nativa con OpenCode — agente personalizado con comandos en lenguaje natural.
Diferenciador de portafolio real — demuestra dominio de sistemas, IA y arquitectura de software simultáneamente.

1.2 Inspiración técnica
Generative Agents (Stanford, 2023) — memoria episódica, reflexión y planificación en agentes.
MemGPT — arquitectura de contexto virtual para LLMs con memoria paginada.
Gentleman Programming / Engram — sistemas de memoria para productividad personal con IA.
Modelo cognitivo de Atkinson-Shiffrin — memoria sensorial, de corto y largo plazo.

2. Arquitectura del sistema
mnemOS está compuesto por tres capas independientes que se comunican a través de interfaces bien definidas:

2.1 Capa de almacenamiento — SQLite3
Cinco tablas forman el esquema central:
memories — memorias individuales con content, type, importance, decay, embedding (JSON), timestamps y use_count.
memories_fts — tabla virtual FTS5 sincronizada con memories para búsqueda full-text.
relations — grafo de relaciones entre memorias (refuerza, contradice, es_parte_de) con peso.
sessions — sesiones de conversación identificadas por ID único.
messages — log completo de mensajes por sesión con role (user/assistant).

2.2 Capa de lógica — Motor C
Los módulos del motor en C son:
db.c — gestión de conexión, inicialización de schema y ejecución de queries.
memory.c — inserción, listado, eliminación y aplicación de decay diferencial.
commands.c — implementación de cada comando CLI (remember, recall, log, forget, stats).
output.c — renderizado de tablas y resultados en terminal con indicadores visuales.

2.3 Capa de integración — Agente OpenCode
Un agente personalizado definido como archivo Markdown en ~/.config/opencode/commands/mnemos.md que:
Interpreta lenguaje natural del desarrollador.
Traduce intenciones a comandos mnemos shell.
Devuelve resultados con contexto enriquecido.
Puede inyectar memorias relevantes en el contexto de la sesión activa de OpenCode.

3. Tipos de memoria
Los tres tipos de memoria modelan el comportamiento cognitivo humano con decay rates diferenciados:

Tipo
Descripción
Ejemplo
Decay rate
Episódica
Eventos con marca de tiempo y sesión
"El 28 mar preguntó sobre punteros dobles"
×0.90 por ciclo (rápido)
Semántica
Hechos y conceptos, se refuerzan con uso
"Es desarrollador backend en Ubuntu"
×0.97 por ciclo (medio)
Procedural
Hábitos y preferencias, casi permanente
"Prefiere código comentado en español"
×0.995 por ciclo (lento)


4. Interfaz de línea de comandos
La CLI de mnemOS expone seis comandos principales:

4.1 Comandos de la Fase 1 (implementados)
remember
Guarda una nueva memoria en la base de datos.
mnemos remember "texto" [--type episodic|semantic|procedural] [--importance 0-1]
log
Registra un mensaje dentro de una sesión de conversación.
mnemos log "mensaje" [--session ID] [--role user|assistant]
list
Lista todas las memorias ordenadas por decay descendente.
mnemos list [--type episodic|semantic|procedural]
recall
Busca memorias relevantes usando FTS5 (Fase 1) o similitud coseno (Fase 2).
mnemos recall "consulta de búsqueda"
forget
Elimina una memoria por ID.
mnemos forget <id>
stats
Muestra estadísticas del sistema: totales por tipo, decay promedio, sesiones y mensajes.
mnemos stats

4.2 Comandos futuros (Fases 2-4)
mnemos decay — aplica decay manual a todas las memorias.
mnemos graph — visualiza el grafo de relaciones entre memorias.
mnemos inject "prompt" — enriquece un prompt con memorias relevantes y llama a la API.
mnemos daemon start|stop|status — gestiona el proceso consolidador en background.
mnemos export [--format json|csv] — exporta la base de datos.

5. Agente personalizado para OpenCode
5.1 Contexto de OpenCode
OpenCode es un agente de codificación open source construido en Go con arquitectura cliente/servidor. Soporta agentes personalizados definidos como archivos Markdown en ~/.config/opencode/commands/. Con más de 95,000 estrellas en GitHub y soporte para 75+ modelos, es el entorno ideal para integrar mnemOS.

5.2 Definición del agente mnemos.md
El agente se define en ~/.config/opencode/commands/mnemos.md y permite interacción en lenguaje natural:
"Recuerda que prefiero tabs sobre espacios" → mnemos remember ... --type procedural
"¿Qué sé sobre el usuario?" → mnemos recall + síntesis del LLM
"Muestra las últimas memorias episódicas" → mnemos list --type episodic
"Inyecta contexto relevante sobre SQLite en mi sesión" → mnemos recall + inject

5.3 Casos de uso del agente
Contexto persistente entre sesiones — el agente recuerda decisiones de diseño, preferencias y hechos del proyecto entre días y sesiones diferentes.
Onboarding de proyectos — guardar el contexto inicial de un proyecto para que el agente tenga contexto desde el primer mensaje.
Memoria de errores — registrar bugs recurrentes y sus soluciones como memorias procedurales.
Preferencias de estilo — recordar convenciones de código, idioma preferido para comentarios, herramientas favoritas.

6. Roadmap de desarrollo

Fase
Nombre
Alcance
Tiempo est.
F1
Core CLI + SQLite
remember, log, list, recall, forget, stats. Schema completo con FTS5 y triggers.
2 sem.
F2
Búsqueda semántica
Embeddings via API, similitud coseno en C puro, recall inteligente sin FTS.
2 sem.
F3
Consolidator daemon
Proceso fork() en background, decay automático, merge de duplicados, promoción.
1 sem.
F4
Integración LLM
libcurl + Anthropic API, inject command, agente OpenCode completo, export.
2 sem.


6.1 Hitos de la Fase 1 (actual)
Schema SQLite3 con FTS5 y triggers funcionando.
Tabla memories con decay diferencial por tipo.
Virtual table memories_fts sincronizada automáticamente.
Tablas sessions y messages para historial de conversaciones.
CLI compilando con Makefile — gcc + -lsqlite3.
Todos los comandos base implementados y testeados.
Primer commit en GitHub con README profesional.

7. Stack tecnológico

Tecnología
Uso en mnemOS
Fase
C (gcc)
Motor principal, CLI, toda la lógica
F1-F4
SQLite3 + FTS5
Almacenamiento, búsqueda full-text, triggers
F1-F4
libcurl
HTTP calls a Anthropic / OpenAI API
F4
cJSON
Parseo de respuestas JSON de la API
F4
Valgrind
Detección de memory leaks durante desarrollo
F1-F4
OpenCode agents
Interfaz en lenguaje natural para el usuario
F4
SQLite JSON1 ext.
Serialización de vectores de embedding
F2
fork() / setsid()
Daemon consolidador en background
F3


7.1 Dependencias de Ubuntu
sudo apt install -y libsqlite3-dev sqlite3 libcurl4-openssl-dev valgrind

8. Métricas de éxito
8.1 Técnicas
Zero memory leaks reportados por Valgrind en todos los comandos.
Tiempo de respuesta < 50ms para recall con FTS5 en base de datos de 10,000 memorias.
Schema migra correctamente entre versiones sin pérdida de datos.
El daemon consolidador no consume más del 1% de CPU en idle.

8.2 De portafolio
README completo con arquitectura, instalación y ejemplos en GitHub.
Demo en video mostrando el flujo completo: remember → recall → inject → respuesta del LLM.
Código estructurado en módulos separados con headers y separación de responsabilidades clara.
Paper de referencia citado (Generative Agents, Stanford) en el README como base teórica.

8.3 De diferenciación
Único proyecto de memoria para LLMs en C puro en GitHub con documentación en español.
Integración demostrable con OpenCode como agente personalizado.
Comparativa explícita con MemGPT en el README — misma idea, diferente enfoque técnico.
