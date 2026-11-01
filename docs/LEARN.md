# Learning guide

1. Mulai dari `run.sh` dan `run-terminal.sh` untuk memahami dua host interface.
2. Baca `wasm/port.cpp` untuk melihat boot minimal dan command-line ABI.
3. Baca `wasm/platform.cpp` untuk storage, console, dan time adapter.
4. Pelajari `kernel/commands.cpp` untuk dispatcher shell.
5. Pelajari `fs/` untuk MyFS/VFS, `process/` untuk scheduler metadata, dan `memory/` untuk page allocator linear-memory.
6. Jalankan `wasm/test_wasm.cjs` dan host tests setelah mengubah subsistem.
