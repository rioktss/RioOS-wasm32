# Portable WASM32 build entry points

The WASM32 source tree follows the same platform-folder convention as the main RioOS repository. Every platform wrapper resolves the repository root automatically.

## Platform layout

- `linux/` — `build.sh`, `run.sh`, `run-terminal.sh`
- `termux/` — `build.sh`, `run.sh`, `run-terminal.sh`
- `macos/` — `build.sh`, `run.sh`, `run-terminal.sh`
- `windows/` — `build.ps1`, `run.ps1`, `run-terminal.ps1`
- `wasm/` — canonical WebAssembly build, Node terminal, browser terminal and framebuffer UI runners

## Dependencies

Required for building: Clang/Clang++, `wasm-ld` (LLD), Python 3.

Required for the host terminal and Node smoke tests: Node.js 18+.

Required for browser mode: a modern WebAssembly-capable browser with Canvas 2D. Persistent browser filesystem uses `localStorage`.

No QEMU or AArch64 cross compiler is needed for the WASM32 target.

## Build commands

```bash
bash linux/build.sh
bash termux/build.sh
bash macos/build.sh
```

Each wrapper delegates to `wasm/build.sh`. Options are `--help`, `--clean`, `--verbose` / `-v`, and `--run`.

Windows PowerShell uses Git Bash or WSL to run the canonical Bash build:

```powershell
.\windows\build.ps1
```

## Runtime commands

```bash
bash linux/run.sh             # Node host terminal
bash linux/run-terminal.sh    # Browser terminal
bash wasm/run-ui.sh            # Browser framebuffer UI
```

`PORT` changes the browser server port, and `BIND` changes its bind address. For example:

```bash
PORT=9000 BIND=127.0.0.1 bash wasm/run-terminal.sh
```

The host terminal stores sectors as JSON in `~/.rioos-wasm32/`. `RIOOS_DATA_DIR` changes that directory. Browser mode stores sectors in the site's `localStorage` and can be affected by browser quota or data-clearing settings.

## Build outputs

```text
wasm/dist/rioos.wasm
wasm/dist/index.html
wasm/dist/ui.html
wasm/dist/obj/*.o
```

`wasm/dist/obj/` is temporary and ignored by version control; the `.wasm` module and HTML pages are runtime artifacts.

## Design boundary

The host OS supplies the compiler, linker, Python, Node.js and browser. The WebAssembly module does not boot as a native kernel and does not have MMU/GIC/IRQ, VirtIO devices, native AArch64 ELF execution, CPU stack switching or SMP. See [WASM32_PORT.md](WASM32_PORT.md).
