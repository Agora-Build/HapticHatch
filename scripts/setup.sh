#!/usr/bin/env bash
# setup.sh — bootstrap the HapticHatch development environment
#
# Clones and installs:
#   ../esp-idf   v5.2.3  (ESP-IDF, required)
#   ../esp-adf   v2.7    (ESP-ADF, optional)
#   ../aosl      master  (Agora OS Abstraction Layer, required)
#
# Usage:
#   ./scripts/setup.sh           # IDF + AOSL only
#   ./scripts/setup.sh --with-adf  # also install ESP-ADF

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPS_DIR="$(cd "$PROJECT_ROOT/.." && pwd)"

IDF_DIR="$DEPS_DIR/esp-idf"
ADF_DIR="$DEPS_DIR/esp-adf"
AOSL_DIR="$DEPS_DIR/aosl"

IDF_VERSION="v5.2.3"
ADF_VERSION="v2.7"

WITH_ADF=0
for arg in "$@"; do
  [[ "$arg" == "--with-adf" ]] && WITH_ADF=1
done

info()  { echo "[setup] $*"; }
ok()    { echo "[setup] ✓ $*"; }
die()   { echo "[setup] ERROR: $*" >&2; exit 1; }

# ── ESP-IDF ──────────────────────────────────────────────────────────────────
if [[ -d "$IDF_DIR" ]]; then
    ok "esp-idf already present at $IDF_DIR"
else
    info "Cloning ESP-IDF $IDF_VERSION → $IDF_DIR"
    git clone --branch "$IDF_VERSION" --depth 1 \
        https://github.com/espressif/esp-idf.git "$IDF_DIR"
fi

info "Installing ESP-IDF tools for esp32s3"
"$IDF_DIR/install.sh" esp32s3
ok "ESP-IDF tools installed"

# ── ESP-ADF (optional) ───────────────────────────────────────────────────────
if [[ "$WITH_ADF" -eq 1 ]]; then
    if [[ -d "$ADF_DIR" ]]; then
        ok "esp-adf already present at $ADF_DIR"
    else
        info "Cloning ESP-ADF $ADF_VERSION → $ADF_DIR"
        git clone --branch "$ADF_VERSION" --depth 1 \
            https://github.com/espressif/esp-adf.git "$ADF_DIR"
        git -C "$ADF_DIR" submodule update --init --recursive
    fi
    ok "ESP-ADF ready"
fi

# ── AOSL ─────────────────────────────────────────────────────────────────────
if [[ -d "$AOSL_DIR" ]]; then
    ok "aosl already present at $AOSL_DIR"
else
    info "Cloning AOSL → $AOSL_DIR"
    git clone https://github.com/AgoraIO-Community/aosl.git "$AOSL_DIR"
fi
ok "AOSL ready"

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "Setup complete. To activate the environment in your shell:"
echo ""
echo "  source $IDF_DIR/export.sh"
echo ""
echo "Then build and flash:"
echo ""
echo "  cd $PROJECT_ROOT"
echo "  rm -f sdkconfig"
echo "  idf.py -p /dev/ttyUSB0 build flash monitor"
