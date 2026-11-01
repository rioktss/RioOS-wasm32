# Roadmap — RioOS WASM32

Roadmap ini fokus pada penguatan port browser tanpa mengklaim ulang fitur hardware AArch64.

## Stabilitas

- Tambah coverage untuk command dispatcher, file descriptor, VFS dan persistence.
- Tambah uji browser otomatis untuk `/`, `/ui.html`, keyboard/touch, resize dan quota storage.
- Perjelas pesan capability pada setiap command yang tidak didukung oleh sandbox.

## Fitur browser

- Tambah export/import image MyFS untuk backup dan restore data.
- Tingkatkan pengelolaan storage ketika localStorage penuh atau diblokir.
- Tambah input accessibility dan skala framebuffer adaptif.
- Tambah konfigurasi tema/wallpaper yang bertahan setelah reload.

## Portability

- Jalankan test pada Chromium, Firefox, dan Safari/WebKit bila toolchain tersedia.
- Tingkatkan integrasi build CI untuk target `wasm32-unknown-unknown`.
- Jaga source portable tetap sinkron dengan baseline MyKernel utama.

## Out of scope untuk browser WASM biasa

Physical AArch64 MMU, GIC/IRQ, VirtIO, AArch64 ELF execution, native kernel thread context switching, dan SMP bukan target dari browser adapter. Fitur tersebut membutuhkan target/emulator/platform native terpisah.
