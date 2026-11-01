# FAQ — RioOS WASM32

## Apakah RioOS WASM32 adalah versi bare-metal?

Bukan. Ini adalah build WebAssembly yang berjalan di dalam browser atau Node.js. Versi bare-metal AArch64 adalah target terpisah.

## Apakah harus memasang QEMU?

Tidak. Build dan run WASM32 tidak menggunakan QEMU. Toolchain yang diperlukan adalah Clang/Clang++, LLD, Python 3, serta Node.js untuk host terminal dan tes.

## Apakah UI pixel lama masih ada?

Ya. Framebuffer 1024×768, lock screen, Home, Control Centre, serta tujuh aplikasi RPK tersedia di `wasm/run-ui.sh` melalui halaman `/ui.html`. Terminal browser berada di `/` melalui `run-terminal.sh`.

## Apakah perintah terminal benar-benar menjalankan kernel?

Command dijalankan oleh command dispatcher C++ di modul WASM melalui `rio_cli_execute()`. Halaman browser dan Node runner hanya menjadi host input/output dan storage bridge.

## Apakah internet, ping, dan HTTP aktif?

Source network stack TCP/UDP/IP disinkronkan, tetapi browser adapter tidak menyediakan raw NIC atau VirtIO-Net. Karena itu jangan menganggap perintah jaringan guest memiliki konektivitas internet.

## Apakah WASM32 mendukung MMU, process switching, thread, atau SMP?

Tidak. Port menyediakan page allocator dan process/scheduler metadata yang portable. Browser sandbox tidak menyediakan physical MMU AArch64, exception level, native CPU stack switching, atau guest multicore.

## Di mana data file disimpan?

Browser terminal dan UI menggunakan `localStorage` browser. Node host terminal memakai `~/.rioos-wasm32/myfs-sectors.json`, atau lokasi yang ditentukan `RIOOS_DATA_DIR`.

## Bagaimana menjalankan test?

Jalankan `bash wasm/build.sh` lalu `node wasm/test_wasm.cjs`. Ini memeriksa build target, boot, framebuffer, navigasi UI, aplikasi bawaan, command dispatcher dan persistence reload. Tes ini tidak menjamin tidak ada bug pada semua lingkungan.
