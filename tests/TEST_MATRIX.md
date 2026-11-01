# MyKernel WASM32 Test Matrix

## Build dan smoke test

```bash
python3 wasm/build.py --clean
node wasm/test_wasm.cjs
```

Strict build memakai C++17 dan `-Wall -Wextra -Werror`. Smoke test memeriksa import/export ABI terminal-only, boot, shell command dispatcher, diagnostics platform, operasi file, perubahan current directory, dan reload MyFS. Tes juga memastikan export framebuffer/UI tidak ada pada build.

## Portable host regression tests

```bash
python3 tests/run_host_tests.py
```

Runner menjalankan 11 test portable untuk page allocator/fault counters, scheduler policy, environment, diagnostics, process lifecycle, scheduler/process integration, syscall compatibility, network stack, MyFS, VFS, dan file descriptors.

## Browser check

```bash
bash run-terminal.sh
```

Pastikan `/` memuat terminal browser, perintah dieksekusi oleh command dispatcher WASM, dan file MyFS tetap ada setelah reload jika browser mempertahankan `localStorage`. Node smoke test tidak menggantikan uji pada browser sebenarnya.
