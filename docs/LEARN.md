# Learning Guide — RioOS WASM32

Urutan yang disarankan untuk memahami source:

1. `wasm/port.cpp` — export API boot, event input, framebuffer dan command-line.
2. `wasm/platform.cpp` — kontrak host untuk storage, clock, terminal, dan capability.
3. `kernel/commands.cpp` — command dispatcher shell.
4. `fs/fs.cpp` dan `fs/vfs.cpp` — struktur MyFS dan virtual filesystem.
5. `kernel/syscall.cpp` dan `kernel/fd.cpp` — syscall compatibility serta file descriptor.
6. `memory/page_alloc.cpp` dan `memory/wasm_memory.cpp` — page allocator generik dan arena linear memory.
7. `process/` — metadata proses dan logical scheduler.
8. `net/netstack.cpp` — implementasi portable stack jaringan; perangkat NIC tidak disediakan browser.
9. `ui/` dan `drivers/fb.cpp` — framebuffer, navigasi, serta aplikasi.
10. `wasm/test_wasm.cjs` dan `tests/` — validasi behavior.

Mulai dari `wasm/build.sh` untuk melihat daftar source yang benar-benar dikompilasi. File assembly AArch64 dan linker native yang dipertahankan sebagai referensi tidak menjadi bagian dari binary WASM.
