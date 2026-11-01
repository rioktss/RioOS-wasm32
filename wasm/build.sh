#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

usage() {
    cat <<'HELP'
RioOS MyKernel WASM32 build

Usage: bash wasm/build.sh [options]

Options:
  --clean       Remove generated object files and old WASM output first
  --verbose, -v Print full compiler/linker commands
  --run         Build, then launch the Node.js host terminal
  --help, -h    Show this help
HELP
}

CLEAN=0
VERBOSE=0
RUN_AFTER=0
for arg in "$@"; do
    case "$arg" in
        --clean) CLEAN=1 ;;
        --verbose|-v) VERBOSE=1 ;;
        --run) RUN_AFTER=1 ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Unknown build option: $arg" >&2; usage >&2; exit 2 ;;
    esac
done

CLANGXX="${CLANGXX:-clang++}"
CLANG="${CLANG:-clang}"
WASM_LD="${WASM_LD:-wasm-ld}"
PYTHON="${PYTHON:-python3}"
OUT="wasm/dist"
OBJ="$OUT/obj"
mkdir -p "$OBJ"
if (( CLEAN )); then
    rm -f "$OBJ"/*.o "$OUT/rioos.wasm"
fi
# Always rebuild object files: the source list is small and this avoids stale ABI objects.
rm -f "$OBJ"/*.o "$OUT/rioos.wasm"

COMMON=(--target=wasm32-unknown-unknown -Iinclude -std=c++17 -O2 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -nostdinc++)
compile() {
    local source="$1" object="$2"; shift 2
    echo "[WASM32] $source"
    if (( VERBOSE )); then
        "$CLANGXX" "${COMMON[@]}" "$@" -c "$source" -o "$OBJ/$object.o"
    else
        "$CLANGXX" "${COMMON[@]}" "$@" -c "$source" -o "$OBJ/$object.o" 2>&1
    fi
}

"$PYTHON" wasm/generate_assets.py

# Portable MyKernel v1.13.0 modules shared with the native source tree.
compile kernel/commands.cpp commands
compile kernel/diag.cpp diag
compile kernel/fault.cpp fault
compile kernel/fd.cpp fd
compile kernel/history.cpp history
compile kernel/env.cpp env
compile kernel/string.cpp string
compile kernel/syscall.cpp syscall -DHOST_TEST
compile kernel/version.cpp version
compile fs/fs.cpp fs
compile fs/vfs.cpp vfs
compile net/netstack.cpp netstack
compile process/process.cpp process
compile process/scheduler.cpp scheduler
compile process/sched_policy.cpp sched_policy
compile memory/page_alloc.cpp page_alloc
compile memory/wasm_memory.cpp wasm_memory

# Browser framebuffer, UI, fonts and application assets.
compile drivers/fb.cpp fb -DHOST_TEST
compile drivers/font.cpp font
compile drivers/ui_font_data.cpp ui_font_data
compile ui/apps.cpp ui_apps
compile ui/boot.cpp ui_boot
compile ui/calculator.cpp ui_calculator
compile ui/clock.cpp ui_clock
compile ui/file_manager.cpp ui_file_manager
compile ui/home_screen.cpp ui_home_screen
compile ui/image.cpp ui_image
compile ui/launcher.cpp ui_launcher
compile ui/lock_screen.cpp ui_lock_screen
compile ui/settings.cpp ui_settings
compile ui/terminal.cpp ui_terminal
compile ui/wallpaper.cpp ui_wallpaper

# Host adapter and WASM exported runtime API.
compile wasm/platform.cpp platform
compile wasm/seed_assets.cpp seed_assets
compile wasm/port.cpp port
if (( VERBOSE )); then
    "$CLANG" --target=wasm32-unknown-unknown -ffreestanding -v -c wasm/generated/assets.S -o "$OBJ/assets.o"
else
    "$CLANG" --target=wasm32-unknown-unknown -ffreestanding -c wasm/generated/assets.S -o "$OBJ/assets.o"
fi

LINK_ARGS=(--no-entry --gc-sections --export-memory --initial-memory=33554432 --max-memory=268435456 \
    --export=rio_init --export=rio_finish_boot --export=rio_key --export=rio_pointer \
    --export=rio_tick --export=rio_framebuffer_ptr --export=rio_framebuffer_width \
    --export=rio_framebuffer_height --export=rio_take_dirty --export=rio_get_ui_state \
    --export=rio_get_active_app --export=rio_seed_assets --export=rio_terminal_selftest_flags \
    --export=rio_cli_buffer_ptr --export=rio_cli_cwd_ptr --export=rio_cli_execute \
    --allow-undefined)
if (( VERBOSE )); then
    "$WASM_LD" -v "${LINK_ARGS[@]}" "$OBJ"/*.o -o "$OUT/rioos.wasm"
else
    "$WASM_LD" "${LINK_ARGS[@]}" "$OBJ"/*.o -o "$OUT/rioos.wasm"
fi

"$PYTHON" wasm/verify_wasm.py "$OUT/rioos.wasm"
cp wasm/index.html "$OUT/index.html"
cp wasm/ui.html "$OUT/ui.html"
"$PYTHON" tests/verify_release.py
echo "WASM32 build success: $OUT/rioos.wasm"

if (( RUN_AFTER )); then
    exec node "$ROOT/wasm/terminal.cjs"
fi
