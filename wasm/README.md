# WASM32 runtime

Lapisan ini khusus untuk target WASM32 terminal-only. Tidak ada halaman UI grafis, framebuffer, paket RPK, atau runner UI.

- `build.py`: builder C++17 yang menghasilkan `rioos.wasm` di root proyek.
- `platform.cpp`: adapter host untuk console, storage sectors, clock, dan fasilitas platform.
- `port.cpp`: boot runtime dan API command-line yang diekspor ke host.
- `terminal.cjs`: terminal host Node.js.
- `test_wasm.cjs`: smoke test modul WebAssembly dan shell.

Halaman browser utama adalah `../index.html` dan sengaja berada di root agar bisa langsung dipasang pada root hosting statis. File `../rioos.wasm` adalah modul yang dimuat halaman tersebut.
