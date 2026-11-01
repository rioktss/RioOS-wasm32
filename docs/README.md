# Dokumentasi RioOS WASM32

Dokumentasi ini menjelaskan port browser MyKernel v1.13.0. Untuk memulai, baca:

- [Build & Run](BUILD_AND_RUN.md) — dependency, perintah, dan runner per platform.
- [Architecture](ARCHITECTURE.md) — susunan modul dan alur runtime WASM.
- [WASM32 Port](WASM32_PORT.md) — matriks fitur yang didukung, adaptasi, dan batas platform.
- [Terminal](TERMINAL.md) — host terminal dan browser terminal.
- [Development](DEVELOPMENT.md) — build, test, dan aturan sinkronisasi source.
- [Learning Guide](LEARN.md) — urutan mempelajari source.
- [Roadmap](ROADMAP.md) — target lanjutan port.
- [Test Matrix](../tests/TEST_MATRIX.md) — test otomatis dan keterbatasannya.

Folder `wasm/` berisi adapter khusus browser, `wasm/build.sh`, terminal browser, halaman framebuffer `ui.html`, dan smoke test. Entry point platform berada di `linux/`, `termux/`, `macos/`, serta `windows/`.
