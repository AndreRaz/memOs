
-- Memorias individuales
CREATE TABLE IF NOT EXISTS memories (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    content     TEXT    NOT NULL,
    type        TEXT    NOT NULL CHECK(type IN ('episodic','semantic','procedural')),
    importance  REAL    NOT NULL DEFAULT 0.5,
    embedding   TEXT,                          -- vector JSON [0.1, 0.3, ...] fase 2
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_used   DATETIME DEFAULT CURRENT_TIMESTAMP,
    use_count   INTEGER DEFAULT 0,
    decay       REAL    DEFAULT 1.0            -- 1.0=fresca, 0.0=olvidada
);

-- Índice full-text search para recall (fase 2)
CREATE VIRTUAL TABLE IF NOT EXISTS memories_fts
    USING fts5(content, content='memories', content_rowid='id');

-- Trigger: mantener FTS sincronizado al insertar
CREATE TRIGGER IF NOT EXISTS memories_ai
    AFTER INSERT ON memories BEGIN
        INSERT INTO memories_fts(rowid, content)
        VALUES (new.id, new.content);
END;

-- Trigger: mantener FTS sincronizado al borrar
CREATE TRIGGER IF NOT EXISTS memories_ad
    AFTER DELETE ON memories BEGIN
        INSERT INTO memories_fts(memories_fts, rowid, content)
        VALUES ('delete', old.id, old.content);
END;

-- Relaciones entre memorias (grafo de conocimiento)
CREATE TABLE IF NOT EXISTS relations (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    source_id   INTEGER NOT NULL REFERENCES memories(id) ON DELETE CASCADE,
    target_id   INTEGER NOT NULL REFERENCES memories(id) ON DELETE CASCADE,
    relation    TEXT    NOT NULL,  -- "refuerza", "contradice", "es_parte_de"
    weight      REAL    DEFAULT 1.0
);

-- Sesiones de conversación
CREATE TABLE IF NOT EXISTS sessions (
    id          TEXT    PRIMARY KEY,           -- UUID o timestamp string
    summary     TEXT,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Mensajes dentro de cada sesión
CREATE TABLE IF NOT EXISTS messages (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    session_id  TEXT    NOT NULL REFERENCES sessions(id) ON DELETE CASCADE,
    role        TEXT    NOT NULL CHECK(role IN ('user','assistant')),
    content     TEXT    NOT NULL,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP
);
