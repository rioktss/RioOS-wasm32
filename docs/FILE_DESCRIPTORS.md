# RioOS file descriptors — portable layer

The shared file-descriptor module is compiled into the WASM32 build. It provides a bounded Unix-inspired descriptor interface, but this browser target does not execute native AArch64 ELF programs.

## Operations

| Operation | Syscall | Helper name in native ABI |
|---|---:|---|
| Open path | `SYS_FD_OPEN` (15) | `user_fd_open(path, flags)` |
| Close descriptor | `SYS_FD_CLOSE` (16) | `user_fd_close(fd)` |
| Read bytes | `SYS_FD_READ` (17) | `user_fd_read(fd, buffer, length)` |
| Write bytes | `SYS_FD_WRITE` (18) | `user_fd_write(fd, buffer, length)` |
| Seek file | `SYS_FD_SEEK` (19) | `user_fd_seek(fd, offset, whence)` |
| Duplicate descriptor | `SYS_FD_DUP` (20) | `user_fd_dup(fd)` |
| Duplicate to slot | `SYS_FD_DUP2` (21) | `user_fd_dup2(old_fd, new_fd)` |
| Create pipe | `SYS_FD_PIPE` (22) | `user_fd_pipe(int descriptors[2])` |

The syscall numbers and flags remain part of the shared header ABI. The browser build routes system calls through a compatibility dispatcher rather than executing an AArch64 `svc` instruction.

## Per-process tables

Each active PID may have its own bounded descriptor table. Descriptors `0`, `1`, and `2` represent standard streams. `dup` and `dup2` share the open-file description and therefore the file offset. Process lifecycle cleanup releases associated tables.

This metadata support does not imply that multiple WASM execution contexts run concurrently. The current browser target has one event loop and no native process context switch.

## Bounded pipes

Pipes are byte streams with a 4096-byte ring buffer. They are non-blocking:

- Empty pipe with a writer: `-11` (`EAGAIN`-style).
- Empty pipe after all writers close: `0` (EOF).
- Full pipe: write returns `-11` when no space remains; partial writes are possible.
- Pipe with no reader: write returns `-32` (`EPIPE`-style).

The shell's text pipeline parser is not a general ELF-to-ELF process pipeline.

## Limits

- 24 descriptor slots per process.
- 32 descriptor tables maximum.
- 128 simultaneous open-file descriptions and 8 pipe objects.
- Maximum read/write request: 4096 bytes per syscall.
- Maximum file size handled by the descriptor layer: 256 KiB.
- Error values are negative errno-like numbers.
- Syscall paths validate user ranges before copying buffers.

## Validation

Run `python3 tests/run_host_tests.py` for the file-descriptor and syscall host regressions. These tests validate the shared module with test doubles; they do not enable native ELF execution in the browser.
