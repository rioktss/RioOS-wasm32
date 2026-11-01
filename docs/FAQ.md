# FAQ

## Bagaimana build MyKernel WASM32?

Pilih skrip platform: `bash termux/build.sh`, `bash linux/build.sh`, atau `bash macos/build.sh`. Untuk Windows PowerShell gunakan `windows/build.ps1`. Pastikan Clang, LLD, dan Python 3 tersedia.

## Bagaimana menjalankan MyKernel WASM32?

Setelah build berhasil, gunakan `bash run.sh` untuk shell terminal host atau `bash run-terminal.sh` untuk terminal browser lokal. Pada Windows gunakan `windows/run.ps1` atau `windows/run-terminal.ps1`.

## Di mana index hosting?

`index.html` berada di root proyek. Upload `index.html` dan `rioos.wasm` ke root hosting yang sama.

## Apakah ada UI atau aplikasi grafis?

Tidak. Edisi ini sengaja terminal-only dan tidak menyertakan framebuffer UI maupun paket aplikasi RPK.

## Apakah browser WASM menyediakan MMU, SMP dan IRQ fisik?

Tidak. WASM berjalan dalam sandbox browser dan tidak memiliki akses ke fasilitas hardware bare-metal AArch64.
