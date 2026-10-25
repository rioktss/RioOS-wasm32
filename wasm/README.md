# RioOS WASM32 Browser Edition

This directory contains the WASM32 source, host adapter, Linux-style browser terminal, build scripts, asset generator and tests. The archive intentionally omits native AArch64 boot code and hardware-specific implementations.

## Build and test

Requirements: Clang/Clang++, LLD (`wasm-ld`), Python 3, Node.js 18+ for the smoke test, and a browser with WebAssembly and Canvas support.

```sh
# Host-terminal mode (interactive shell in Termux)
bash run.sh

# Graphical UI mode (opens browser)
bash run-ui.sh

# Rebuild WASM32 from source and run regression tests
RIOOS_REBUILD=1 bash run.sh   # rebuild and enter terminal mode
bash wasm/build.sh            # rebuild only
node wasm/test_wasm.cjs
```

Run `PORT=9000 bash run-ui.sh` to select another port. The browser page uses a minimal terminal-inspired layout around RioOS’s graphical framebuffer. `bash run.sh` instead starts a real line-oriented host terminal connected to RioOS’s WASM32 command dispatcher and persistent MyFS image.

## Ported behavior

- Preserves RioOS's pixel UI renderer, lock screen, Home screen and seven built-in app modules inside the WASM build for source/feature continuity. The current browser page intentionally exposes the terminal interface rather than the framebuffer UI.
- Reuses the shared shell command dispatcher, environment variables, shell history, file descriptors, VFS/MyFS and built-in app manifests.
- The browser terminal uses a Linux-like prompt, command transcript, command history, keyboard shortcuts, and clear/copy/restart controls.
- Commands are executed by RioOS’s actual `rio_cli_execute` API inside the WASM module, not by a fake JavaScript command simulator.
- MyFS sector data is bridged to browser `localStorage`, subject to browser quota and site-data policies. In host-terminal mode, MyFS sectors persist at `~/.rioos-wasm32/myfs-sectors.json` (or `RIOOS_DATA_DIR` if set).

## Limitations

WASM browser sandboxing does not expose the native AArch64 MMU, exception vectors, GIC/timer interrupts, privileged execution, native scheduler/context switching/SMP, AArch64 ELF execution or QEMU VirtIO devices. Compatibility APIs are implemented as stubs or report unavailable capabilities. This build is not a bit-for-bit native-kernel replacement and should not be treated as a security boundary.

## Validation

`node wasm/test_wasm.cjs` runs deterministic smoke tests for module boot, framebuffer rendering, lock-to-home navigation, all seven apps, terminal input and filesystem persistence reload. These checks do not guarantee freedom from all bugs.
