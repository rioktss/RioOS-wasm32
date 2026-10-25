#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
CLANGXX="${CLANGXX:-clang++}"
CLANG="${CLANG:-clang}"
WASM_LD="${WASM_LD:-wasm-ld}"
OUT="wasm/dist"
OBJ="$OUT/obj"
mkdir -p "$OBJ"
rm -f "$OBJ"/*.o "$OUT/rioos.wasm"

COMMON=(--target=wasm32-unknown-unknown -Iinclude -std=c++17 -O2 -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -nostdinc++)
compile() {
  local src="$1" name="$2"; shift 2
  echo "[WASM32] $src"
  "$CLANGXX" "${COMMON[@]}" "$@" -c "$src" -o "$OBJ/$name.o"
}

python3 wasm/generate_assets.py
compile kernel/commands.cpp commands
compile fs/fs.cpp fs
compile fs/vfs.cpp vfs
compile kernel/string.cpp string
compile kernel/history.cpp history
compile kernel/env.cpp env
compile kernel/version.cpp version
compile kernel/fd.cpp fd
compile kernel/syscall.cpp syscall -DHOST_TEST
compile net/netstack.cpp netstack
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
compile wasm/platform.cpp platform
compile wasm/seed_assets.cpp seed_assets
compile wasm/port.cpp port
"$CLANG" --target=wasm32-unknown-unknown -ffreestanding -c wasm/generated/assets.S -o "$OBJ/assets.o"

"$WASM_LD" --no-entry --gc-sections --export-memory --initial-memory=33554432 --max-memory=268435456 \
  --export=rio_init --export=rio_finish_boot --export=rio_key --export=rio_pointer \
  --export=rio_tick --export=rio_framebuffer_ptr --export=rio_framebuffer_width \
  --export=rio_framebuffer_height --export=rio_take_dirty --export=rio_get_ui_state \
  --export=rio_get_active_app --export=rio_seed_assets --export=rio_terminal_selftest_flags \
  --export=rio_cli_buffer_ptr --export=rio_cli_cwd_ptr --export=rio_cli_execute --allow-undefined \
  "$OBJ"/*.o -o "$OUT/rioos.wasm"

python3 wasm/verify_wasm.py "$OUT/rioos.wasm"
cp wasm/index.html "$OUT/index.html"
echo "WASM32 build success: $OUT/rioos.wasm"
