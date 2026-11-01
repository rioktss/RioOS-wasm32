#!/usr/bin/env python3
"""Verify the assembled RioOS WASM32 release layout and browser ABI."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT / "wasm" / "dist"
WASM = DIST / "rioos.wasm"
REQUIRED_FILES = [
    ROOT / "README.md",
    ROOT / "docs" / "BUILD_AND_RUN.md",
    ROOT / "docs" / "ARCHITECTURE.md",
    ROOT / "docs" / "WASM32_PORT.md",
    ROOT / "wasm" / "build.sh",
    DIST / "index.html",
    DIST / "ui.html",
    WASM,
]
REQUIRED_DIRS = ["boot", "drivers", "include", "kernel", "memory", "process", "fs", "net", "storage", "tests", "tools", "userspace", "wasm", "linux", "termux", "macos", "windows"]
PLATFORM_SCRIPTS = {
    "linux": ("build.sh", "run.sh", "run-terminal.sh"),
    "termux": ("build.sh", "run.sh", "run-terminal.sh"),
    "macos": ("build.sh", "run.sh", "run-terminal.sh"),
    "windows": ("build.ps1", "run.ps1", "run-terminal.ps1"),
}
APPS = ("calculator", "clock", "files", "launcher", "pengaturan", "terminal", "wallpaper")

missing = [str(path.relative_to(ROOT)) for path in REQUIRED_FILES if not path.is_file()]
missing.extend(name + "/" for name in REQUIRED_DIRS if not (ROOT / name).is_dir())
for folder, scripts in PLATFORM_SCRIPTS.items():
    for script in scripts:
        path = ROOT / folder / script
        if not path.is_file():
            missing.append(str(path.relative_to(ROOT)))
for app in APPS:
    path = ROOT / "userspace" / "apps" / (app + ".rpk")
    if not path.is_file() or path.stat().st_size == 0:
        missing.append(str(path.relative_to(ROOT)))
if missing:
    print("Release verification failed; missing/empty items:", file=sys.stderr)
    for item in missing:
        print(" - " + item, file=sys.stderr)
    raise SystemExit(1)

blob = WASM.read_bytes()
if len(blob) < 8 or blob[:8] != b"\x00asm\x01\x00\x00\x00":
    raise SystemExit("Release verification failed: invalid WebAssembly binary header")
for export in (b"rio_init", b"rio_finish_boot", b"rio_cli_execute", b"rio_framebuffer_ptr", b"rio_terminal_selftest_flags"):
    if export not in blob:
        raise SystemExit(f"Release verification failed: missing WASM ABI export {export.decode()}")

print(f"Release layout verified: {len(REQUIRED_DIRS)} module/platform directories, {len(APPS)} apps, {len(blob)}-byte WASM module")
