#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PYTHON_BIN="${PYTHON:-python3}"
command -v "$PYTHON_BIN" >/dev/null 2>&1 || {
  echo "Missing dependency: Python 3 (set PYTHON=/path/to/python3 if needed)." >&2
  exit 1
}
command -v "${CLANGXX:-clang++}" >/dev/null 2>&1 || {
  echo "Missing dependency: clang++ (install LLVM/Clang)." >&2
  exit 1
}
command -v "${WASM_LD:-wasm-ld}" >/dev/null 2>&1 || {
  echo "Missing dependency: wasm-ld (install LLVM LLD)." >&2
  exit 1
}
cd "$ROOT"
exec "$PYTHON_BIN" wasm/build.py "$@"
