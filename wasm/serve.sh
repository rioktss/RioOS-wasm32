#!/usr/bin/env sh
set -eu
PORT="${PORT:-8080}"
BIND="${BIND:-127.0.0.1}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
DIST="$SCRIPT_DIR/dist"
if [ ! -f "$DIST/index.html" ] || [ ! -f "$DIST/rioos.wasm" ]; then
  echo "ERROR: WASM files not found in $DIST; run ./wasm/build.sh first." >&2
  exit 1
fi
command -v python3 >/dev/null 2>&1 || { echo "ERROR: python3 is required to serve RioOS." >&2; exit 1; }
printf 'RioOS WASM32: http://%s:%s/\nPress Ctrl+C to stop.\n' "$BIND" "$PORT"
exec python3 -m http.server "$PORT" --bind "$BIND" --directory "$DIST"
