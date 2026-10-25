# RioOS WASM32 Source Edition

This archive contains the browser/WebAssembly edition of RioOS and only the shared source files needed to build it. Native AArch64 boot code, AArch64 assembly, the native linker script, native-only hardware drivers, QEMU disk images, and the original native build scripts are intentionally excluded.

## What is included

- `wasm/`: WASM32 platform compatibility layer, UI bridge, asset embedding, Linux-style browser terminal (`index.html`), build/serve scripts, verification and smoke-test scripts.
- `kernel/`, `fs/`, `net/`, `drivers/`, `ui/`: only shared C++ modules used by the WASM32 build (shell/commands, environment/history, MyFS/VFS, framebuffer/font renderer, applications, and UI).
- `include/`: the transitive set of headers required to compile those modules. Some headers describe platform APIs that are implemented as browser compatibility stubs in `wasm/platform.cpp`; native driver implementations are not included.
- `apps/`, `assets/`: application manifests and source assets embedded into the WASM module.
- `wasm/dist/`: ready-to-run web output (`index.html` and the compiled `rioos.wasm`).

## Build and test

Requirements: Clang/Clang++, `wasm-ld`, Python 3, and Node.js 18+ for the smoke test.

```sh
# Host-terminal mode (interactive shell in Termux)
bash run.sh

# Linux-style browser terminal
bash run-ui.sh

# Rebuild WASM32 from source and run regression tests
RIOOS_REBUILD=1 bash run.sh   # rebuild, then enter terminal mode
bash wasm/build.sh            # rebuild only
node wasm/test_wasm.cjs
```

The browser launcher serves a Linux-style interactive terminal at `http://127.0.0.1:8080`; command input in the page calls RioOS’s actual WASM command dispatcher and stores MyFS sectors in browser `localStorage`. The browser page is a terminal interface, not the pixel-framebuffer UI. `bash run.sh` remains the separate interactive terminal inside Termux and stores MyFS in `~/.rioos-wasm32/myfs-sectors.json` by default (`RIOOS_DATA_DIR` can override that location). On Android, use Termux with Clang, LLD, Python, and Node.js installed. Browser persistence is subject to site-data and quota policies.

## Port limitations

This is a browser WASM port, not an emulation of the AArch64 machine. Browser WASM cannot provide physical MMU/GIC interrupts, AArch64 exception vectors, native context switching/SMP, AArch64 ELF execution, or QEMU VirtIO devices. Platform-dependent commands are mapped to compatibility behavior or report that the capability is unavailable. Network-stack source is retained as a shared component, but this browser build does not attach a real VirtIO network device.

## Validation

`bash wasm/build.sh` compiles a fresh `wasm/dist/rioos.wasm`, and `node wasm/test_wasm.cjs` runs deterministic smoke tests for module boot, framebuffer output, UI navigation, apps, terminal input, and persistent filesystem reload. These tests do not prove the absence of every possible bug or replace testing in the target browser.
