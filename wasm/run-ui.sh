#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
command -v python3 >/dev/null 2>&1 || { echo "Missing dependency: python3" >&2; exit 1; }
WASM="$ROOT/wasm/dist/rioos.wasm"
if [[ ! -f "$WASM" || "${RIOOS_REBUILD:-0}" == "1" ]]; then
    for tool in clang clang++ wasm-ld python3; do
        command -v "$tool" >/dev/null 2>&1 || { echo "Missing build dependency: $tool" >&2; exit 1; }
    done
    bash "$ROOT/wasm/build.sh"
fi
cp "$ROOT/wasm/index.html" "$ROOT/wasm/dist/index.html"
cp "$ROOT/wasm/ui.html" "$ROOT/wasm/dist/ui.html"
PORT="${PORT:-8080}"
BIND="${BIND:-127.0.0.1}"
URL="http://${BIND}:${PORT}/ui.html"
python3 -m http.server "$PORT" --bind "$BIND" --directory "$ROOT/wasm/dist" &
SERVER_PID=$!
cleanup() { kill "$SERVER_PID" 2>/dev/null || true; wait "$SERVER_PID" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
sleep 1
printf '\nRioOS WASM32 framebuffer UI: %s\n' "$URL"
printf 'Browser terminal: http://%s:%s/\n' "$BIND" "$PORT"
printf 'Keep this session open; press Ctrl+C to stop the server.\n'
if command -v termux-open-url >/dev/null 2>&1; then
    termux-open-url "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$URL" >/dev/null 2>&1 || echo "Open this URL manually: $URL"
else
    echo "Open this URL in your browser: $URL"
fi
wait "$SERVER_PID"
