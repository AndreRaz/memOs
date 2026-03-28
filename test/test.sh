#!/usr/bin/env bash
# =============================================================================
# test/test.sh — Suite de pruebas para mnemOS Fase 1
#
# Prueba los 6 comandos CLI contra una base de datos temporal.
# Uso:  bash test/test.sh  (desde la raíz del proyecto)
#       make test
# =============================================================================

set -euo pipefail

BINARY="./mnemos"
TESTDB="/tmp/mnemos_test.db"
PASS=0
FAIL=0
TOTAL=0

# ── colores ──────────────────────────────────────────────────────────────────
GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
RESET='\033[0m'

# ── helpers ───────────────────────────────────────────────────────────────────
assert_pass() {
    local desc="$1"; shift
    TOTAL=$((TOTAL+1))
    if "$@" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓ PASS${RESET}  $desc"
        PASS=$((PASS+1))
    else
        echo -e "  ${RED}✗ FAIL${RESET}  $desc"
        FAIL=$((FAIL+1))
    fi
}

assert_output_contains() {
    local desc="$1"
    local pattern="$2"; shift 2
    TOTAL=$((TOTAL+1))
    local output
    output=$("$@" 2>&1) || true
    if echo "$output" | grep -q "$pattern"; then
        echo -e "  ${GREEN}✓ PASS${RESET}  $desc"
        PASS=$((PASS+1))
    else
        echo -e "  ${RED}✗ FAIL${RESET}  $desc  (buscando: '$pattern')"
        echo -e "         output: $output"
        FAIL=$((FAIL+1))
    fi
}

assert_output_not_contains() {
    local desc="$1"
    local pattern="$2"; shift 2
    TOTAL=$((TOTAL+1))
    local output
    output=$("$@" 2>&1) || true
    if ! echo "$output" | grep -qF "$pattern"; then
        echo -e "  ${GREEN}✓ PASS${RESET}  $desc"
        PASS=$((PASS+1))
    else
        echo -e "  ${RED}✗ FAIL${RESET}  $desc  (no debería contener: '$pattern')"
        FAIL=$((FAIL+1))
    fi
}

# Sobrescribir DB de datos para usar la de test
export MNEMOS_DB="$TESTDB"

run() {
    "$BINARY" "$@"
}

# ── preparación ───────────────────────────────────────────────────────────────
echo ""
echo -e "${CYAN}══════════════════════════════════════════${RESET}"
echo -e "${CYAN}  mnemOS — Suite de Pruebas Fase 1        ${RESET}"
echo -e "${CYAN}══════════════════════════════════════════${RESET}"
echo ""

# Compilar si el binario no existe
if [ ! -f "$BINARY" ]; then
    echo "  → Compilando..."
    make -s || { echo "Error de compilación"; exit 1; }
fi

# Limpiar DB de test anterior
rm -f "$TESTDB"

echo -e "  DB de prueba: $TESTDB"
echo ""

# =============================================================================
# TEST 1: remember — guardar memorias de los 3 tipos
# =============================================================================
echo -e "${CYAN}─── remember ────────────────────────────────${RESET}"

assert_output_contains \
    "remember tipo semantic (default)" \
    "semantic" \
    run remember "El proyecto usa SQLite3 como motor de base de datos"

assert_output_contains \
    "remember con --type episodic" \
    "episodic" \
    run remember "Hoy resolví el bug de FTS5 con los triggers" --type episodic

assert_output_contains \
    "remember con --type procedural" \
    "procedural" \
    run remember "Prefiero comentarios en español en el código fuente" --type procedural

assert_output_contains \
    "remember con --importance 0.9" \
    "0.9" \
    run remember "SQLite FTS5 requiere sincronización manual con triggers" --type semantic --importance 0.9

# =============================================================================
# TEST 2: list — listar memorias
# =============================================================================
echo ""
echo -e "${CYAN}─── list ────────────────────────────────────${RESET}"

assert_output_contains \
    "list muestra memorias guardadas" \
    "SQLite3" \
    run list

assert_output_contains \
    "list --type episodic filtra correctamente" \
    "episodic" \
    run list --type episodic

assert_output_not_contains \
    "list --type episodic no muestra semantic" \
    "semantic" \
    run list --type episodic

assert_output_contains \
    "list --type procedural muestra preferencias" \
    "procedural" \
    run list --type procedural

# =============================================================================
# TEST 3: recall — búsqueda FTS5
# =============================================================================
echo ""
echo -e "${CYAN}─── recall ──────────────────────────────────${RESET}"

assert_output_contains \
    "recall encuentra memoria por palabra clave" \
    "SQLite" \
    run recall "SQLite"

assert_output_contains \
    "recall encuentra memoria episodica" \
    "bug" \
    run recall "bug"

assert_output_contains \
    "recall con término no encontrado muestra mensaje vacío" \
    "sin resultados" \
    run recall "xyzzy_no_existe_1234"

# =============================================================================
# TEST 4: log — registrar mensajes de sesión
# =============================================================================
echo ""
echo -e "${CYAN}─── log ─────────────────────────────────────${RESET}"

assert_output_contains \
    "log registra mensaje con sesión por defecto" \
    "default" \
    run log "Revisando el módulo de memoria"

assert_output_contains \
    "log registra con sesión personalizada" \
    "sesion-01" \
    run log "Implementando decay diferencial" --session sesion-01

assert_output_contains \
    "log registra rol assistant" \
    "assistant" \
    run log "El bug era un doble free en finalize" --session sesion-01 --role assistant

# =============================================================================
# TEST 5: stats — estadísticas
# =============================================================================
echo ""
echo -e "${CYAN}─── stats ───────────────────────────────────${RESET}"

assert_output_contains \
    "stats muestra tipos de memoria" \
    "semantic" \
    run stats

assert_output_contains \
    "stats muestra sesiones registradas" \
    "Sesiones" \
    run stats

assert_output_contains \
    "stats muestra mensajes en log" \
    "Mensajes" \
    run stats

# =============================================================================
# TEST 6: forget — eliminar memoria
# =============================================================================
echo ""
echo -e "${CYAN}─── forget ──────────────────────────────────${RESET}"

# La memoria con id=1 fue la primera que guardamos
assert_output_contains \
    "forget elimina memoria existente" \
    "eliminada" \
    run forget 1

assert_output_not_contains \
    "list ya no muestra la memoria eliminada (id=1)" \
    "[  1]" \
    run list

# =============================================================================
# RESUMEN
# =============================================================================
echo ""
echo -e "${CYAN}══════════════════════════════════════════${RESET}"

if [ "$FAIL" -eq 0 ]; then
    echo -e "${GREEN}  ✓ Todos los tests pasaron: $PASS/$TOTAL${RESET}"
else
    echo -e "${RED}  ✗ Fallos: $FAIL/$TOTAL  —  Pasados: $PASS/$TOTAL${RESET}"
fi

echo -e "${CYAN}══════════════════════════════════════════${RESET}"
echo ""

# Limpiar DB temporal
rm -f "$TESTDB"

[ "$FAIL" -eq 0 ] && exit 0 || exit 1
