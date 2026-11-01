#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
CXX="${CXX:-clang++}"
OUT="${HOST_TEST_OUT:-$ROOT/build/host-tests}"
mkdir -p "$OUT" "$ROOT/build"
COMMON=(-std=c++17 -O2 -Wall -Wextra -Werror -iquote include -DHOST_TEST)
run_test() {
    local name="$1"; shift
    echo "[HOST TEST] $name"
    "$CXX" "${COMMON[@]}" "$@" -o "$OUT/$name"
    "$OUT/$name"
}

run_test mem_host_test tests/mem_host_test.cpp memory/page_alloc.cpp kernel/fault.cpp
run_test sched_host_test tests/sched_host_test.cpp process/sched_policy.cpp
run_test env_host_test tests/env_host_test.cpp kernel/env.cpp kernel/string.cpp
run_test diag_host_test tests/diag_host_test.cpp kernel/diag.cpp
run_test process_host_test tests/process_host_test.cpp process/process.cpp kernel/string.cpp
run_test scheduler_process_host_test tests/scheduler_process_host_test.cpp process/scheduler.cpp process/process.cpp kernel/string.cpp
run_test syscall_host_test tests/syscall_host_test.cpp kernel/syscall.cpp kernel/fd.cpp
run_test net_host_test tests/net_host_test.cpp net/netstack.cpp
run_test fs_host_test tests/fs_host_test.cpp fs/fs.cpp kernel/string.cpp kernel/rtc.cpp
run_test vfs_host_test tests/vfs_host_test.cpp fs/vfs.cpp fs/fs.cpp kernel/string.cpp kernel/rtc.cpp
run_test fd_host_test tests/fd_host_test.cpp kernel/fd.cpp fs/fs.cpp kernel/string.cpp kernel/rtc.cpp
run_test fb_host_test tests/fb_host_test.cpp drivers/fb.cpp drivers/font.cpp drivers/ui_font_data.cpp
run_test ui_host_test tests/ui_host_test.cpp drivers/fb.cpp drivers/font.cpp drivers/ui_font_data.cpp ui/boot.cpp ui/lock_screen.cpp ui/home_screen.cpp
run_test ui_image_host_test tests/ui_image_host_test.cpp drivers/fb.cpp drivers/font.cpp drivers/ui_font_data.cpp ui/image.cpp ui/wallpaper.cpp ui/settings.cpp kernel/string.cpp

echo "All 14 portable host regression tests passed."
