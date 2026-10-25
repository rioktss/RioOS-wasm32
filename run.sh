#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

command -v node >/dev/null 2>&1 || {
  echo "Missing dependency: nodejs" >&2
  echo "In Termux run: pkg install nodejs -y" >&2
  exit 1
}
WASM="$ROOT/wasm/dist/rioos.wasm"
if [[ ! -f "$WASM" || "${RIOOS_REBUILD:-0}" == "1" ]]; then
  for tool in clang clang++ wasm-ld python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
      echo "Missing build dependency: $tool" >&2
      echo "In Termux run: pkg install clang lld python -y" >&2
      exit 1
    fi
  done
  bash "$ROOT/wasm/build.sh"
fi
exec node "$ROOT/wasm/terminal.cjs"
