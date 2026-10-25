#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

command -v python3 >/dev/null 2>&1 || {
  echo "Missing dependency: python3" >&2
  echo "In Termux run: pkg install python -y" >&2
  exit 1
}
WASM="$ROOT/wasm/dist/rioos.wasm"
if [[ ! -f "$WASM" || "${RIOOS_REBUILD:-0}" == "1" ]]; then
  for tool in clang clang++ wasm-ld; do
    if ! command -v "$tool" >/dev/null 2>&1; then
      echo "Missing build dependency: $tool" >&2
      echo "In Termux run: pkg install clang lld -y" >&2
      exit 1
    fi
  done
  bash "$ROOT/wasm/build.sh"
fi
cp "$ROOT/wasm/index.html" "$ROOT/wasm/dist/index.html"

PORT="${PORT:-8080}"
BIND="${BIND:-127.0.0.1}"
URL="http://${BIND}:${PORT}/"
python3 -m http.server "$PORT" --bind "$BIND" --directory "$ROOT/wasm/dist" &
SERVER_PID=$!
cleanup() { kill "$SERVER_PID" 2>/dev/null || true; wait "$SERVER_PID" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
sleep 1
printf '\nRioOS WASM32 UI is serving at %s\n' "$URL"
printf 'Keep this Termux session open. Press Ctrl+C to stop the UI server.\n'
if command -v termux-open-url >/dev/null 2>&1; then
  termux-open-url "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
elif command -v xdg-open >/dev/null 2>&1; then
  xdg-open "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
else
  echo "Open this URL in your browser: $URL"
fi
wait "$SERVER_PID"
