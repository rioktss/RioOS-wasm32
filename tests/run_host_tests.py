#!/usr/bin/env python3
"""Compile and run the portable MyKernel host regression tests."""
from pathlib import Path
import os
import shlex
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
OUT = Path(os.environ.get("HOST_TEST_OUT", str(ROOT / "build" / "host-tests")))
CXX = shlex.split(os.environ.get("CXX", "clang++"))
COMMON = ["-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror", "-iquote", "include", "-DHOST_TEST"]
TESTS = [
    ("mem_host_test", "tests/mem_host_test.cpp", "memory/page_alloc.cpp", "kernel/fault.cpp"),
    ("sched_host_test", "tests/sched_host_test.cpp", "process/sched_policy.cpp"),
    ("env_host_test", "tests/env_host_test.cpp", "kernel/env.cpp", "kernel/string.cpp"),
    ("diag_host_test", "tests/diag_host_test.cpp", "kernel/diag.cpp"),
    ("process_host_test", "tests/process_host_test.cpp", "process/process.cpp", "kernel/string.cpp"),
    ("scheduler_process_host_test", "tests/scheduler_process_host_test.cpp", "process/scheduler.cpp", "process/process.cpp", "kernel/string.cpp"),
    ("syscall_host_test", "tests/syscall_host_test.cpp", "kernel/syscall.cpp", "kernel/fd.cpp"),
    ("net_host_test", "tests/net_host_test.cpp", "net/netstack.cpp"),
    ("fs_host_test", "tests/fs_host_test.cpp", "fs/fs.cpp", "kernel/string.cpp", "kernel/rtc.cpp"),
    ("vfs_host_test", "tests/vfs_host_test.cpp", "fs/vfs.cpp", "fs/fs.cpp", "kernel/string.cpp", "kernel/rtc.cpp"),
    ("fd_host_test", "tests/fd_host_test.cpp", "kernel/fd.cpp", "fs/fs.cpp", "kernel/string.cpp", "kernel/rtc.cpp"),
]

def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    for name, *sources in TESTS:
        print(f"[HOST TEST] {name}", flush=True)
        command = CXX + COMMON + list(sources) + ["-o", str(OUT / name)]
        subprocess.run(command, cwd=ROOT, check=True)
        subprocess.run([str(OUT / name)], cwd=ROOT, check=True)
    print(f"All {len(TESTS)} portable host regression tests passed.")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        raise SystemExit(error.returncode or 1)
