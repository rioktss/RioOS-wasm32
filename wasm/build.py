#!/usr/bin/env python3
"""Build the terminal-only RioOS WASM32 runtime into ./rioos.wasm."""
from __future__ import annotations
import os
from pathlib import Path
import shlex
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
OBJ = ROOT / "build" / "wasm32" / "obj"
OUTPUT = ROOT / "rioos.wasm"
CLANGXX = shlex.split(os.environ.get("CLANGXX", "clang++"))
WASM_LD = shlex.split(os.environ.get("WASM_LD", "wasm-ld"))

COMMON = [
    "--target=wasm32-unknown-unknown", "-Iinclude", "-std=c++17", "-O2",
    "-Wall", "-Wextra", "-Werror", "-ffreestanding", "-fno-builtin",
    "-fno-exceptions", "-fno-rtti", "-fno-stack-protector",
    "-fno-unwind-tables", "-fno-asynchronous-unwind-tables",
    "-ffunction-sections", "-fdata-sections", "-nostdinc++",
]
SOURCES = [
    ("kernel/commands.cpp", "commands", []),
    ("kernel/diag.cpp", "diag", []),
    ("kernel/fault.cpp", "fault", []),
    ("kernel/fd.cpp", "fd", []),
    ("kernel/history.cpp", "history", []),
    ("kernel/env.cpp", "env", []),
    ("kernel/string.cpp", "string", []),
    ("kernel/syscall.cpp", "syscall", ["-DHOST_TEST"]),
    ("kernel/version.cpp", "version", []),
    ("fs/fs.cpp", "fs", []),
    ("fs/vfs.cpp", "vfs", []),
    ("net/netstack.cpp", "netstack", []),
    ("process/process.cpp", "process", []),
    ("process/scheduler.cpp", "scheduler", []),
    ("process/sched_policy.cpp", "sched_policy", []),
    ("memory/page_alloc.cpp", "page_alloc", []),
    ("memory/wasm_memory.cpp", "wasm_memory", []),
    ("wasm/platform.cpp", "platform", []),
    ("wasm/port.cpp", "port", []),
]


def run(command: list[str]) -> None:
    print("[WASM32] " + " ".join(shlex.quote(part) for part in command[:4]))
    subprocess.run(command, cwd=ROOT, check=True)


def main() -> int:
    if "--help" in sys.argv or "-h" in sys.argv:
        print("Usage: python3 wasm/build.py [--clean] [--verbose]")
        return 0
    clean = "--clean" in sys.argv
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    OBJ.mkdir(parents=True, exist_ok=True)
    if clean:
        for old in OBJ.glob("*.o"):
            old.unlink()
        OUTPUT.unlink(missing_ok=True)
    objects = []
    for source, name, extra in SOURCES:
        src = ROOT / source
        if not src.is_file():
            raise SystemExit(f"Missing WASM source: {source}")
        obj = OBJ / f"{name}.o"
        command = CLANGXX + COMMON + extra + ["-c", source, "-o", str(obj)]
        if verbose:
            print("$ " + " ".join(shlex.quote(part) for part in command))
        run(command)
        objects.append(str(obj))

    link_args = [
        "--no-entry", "--gc-sections", "--export-memory",
        "--initial-memory=33554432", "--max-memory=268435456",
        "--export=rio_init", "--export=rio_finish_boot",
        "--export=rio_cli_buffer_ptr", "--export=rio_cli_cwd_ptr",
        "--export=rio_cli_execute", "--allow-undefined",
    ]
    command = WASM_LD + link_args + objects + ["-o", str(OUTPUT)]
    if verbose:
        print("$ " + " ".join(shlex.quote(part) for part in command))
    run(command)
    data = OUTPUT.read_bytes()
    if len(data) < 8 or data[:8] != b"\0asm\x01\0\0\0":
        OUTPUT.unlink(missing_ok=True)
        raise SystemExit("Build produced an invalid WebAssembly module")
    print(f"WASM32 build success: {OUTPUT.relative_to(ROOT)} ({len(data)} bytes)")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        raise SystemExit(error.returncode or 1)
