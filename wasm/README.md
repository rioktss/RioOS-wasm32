# WASM32 Platform Layer

Folder ini berisi adapter browser/Node, build target wasm32, terminal browser, framebuffer browser UI, runner, dan smoke test.

- `build.sh` — compile source bersama MyKernel v1.13.0 dan adapter WASM, lalu hasilkan `dist/rioos.wasm` serta halaman `dist/index.html` / `dist/ui.html`.
- `platform.cpp` — host compatibility layer (storage sectors, clock, console output, dan capability status).
- `port.cpp` — exported boot, input, framebuffer, dan CLI APIs.
- `index.html` — browser terminal; command dieksekusi oleh dispatcher C++ di WASM.
- `ui.html` — framebuffer 1024×768, lock screen, Home dan aplikasi di Canvas.
- `terminal.cjs` — host terminal Node.js yang memakai module WASM dan persistent JSON sector store.
- `test_wasm.cjs` — integration smoke tests.

## Run

```bash
bash wasm/build.sh
node wasm/test_wasm.cjs
bash wasm/run.sh          # Host terminal Node.js
bash wasm/run-terminal.sh # Browser terminal
bash wasm/run-ui.sh        # Browser framebuffer UI
```

Gunakan `PORT=9000` untuk port berbeda. `BIND=0.0.0.0` dapat dipakai bila halaman ingin diakses dari perangkat lain pada jaringan lokal; jangan expose server ke jaringan yang tidak dipercaya.

WASM port tidak menyediakan hardware AArch64, VirtIO, MMU/GIC, native context switching atau SMP. Rincian ada di `docs/WASM32_PORT.md`.
