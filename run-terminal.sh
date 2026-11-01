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
Lalu jalankan kembali: bash run-terminal.sh
MSG
  exit 1
fi
command -v python3 >/dev/null 2>&1 || { echo "Missing dependency: Python 3" >&2; exit 1; }
PORT="${PORT:-8080}"
BIND="${BIND:-127.0.0.1}"
URL="http://${BIND}:${PORT}/"
python3 -m http.server "$PORT" --bind "$BIND" --directory "$ROOT" &
SERVER_PID=$!
cleanup() { kill "$SERVER_PID" 2>/dev/null || true; wait "$SERVER_PID" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
sleep 1
printf '\nRioOS WASM32 terminal: %s\n' "$URL"
printf 'Hosting root: %s\n' "$ROOT"
printf 'Keep this session open; press Ctrl+C to stop the server.\n'
if command -v termux-open-url >/dev/null 2>&1; then
  termux-open-url "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
elif command -v xdg-open >/dev/null 2>&1; then
  xdg-open "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
else
  echo "Open this URL in your browser: $URL"
fi
wait "$SERVER_PID"
