#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"
if [[ ! -f "$ROOT/rioos.wasm" ]]; then
  cat >&2 <<'MSG'
MyKernel WASM32 belum dibuild.
Build terlebih dahulu sesuai platform:
  Termux : bash termux/build.sh
  Linux  : bash linux/build.sh
  macOS  : bash macos/build.sh
Lalu jalankan kembali: bash run.sh
MSG
  exit 1
fi
command -v node >/dev/null 2>&1 || { echo "Missing dependency: Node.js" >&2; exit 1; }
exec node "$ROOT/wasm/terminal.cjs" "$@"
