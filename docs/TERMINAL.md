# Terminal RioOS

MyKernel WASM32 hanya menyediakan shell teks. Browser terminal berada di `index.html` pada root proyek; host terminal Node.js dijalankan melalui `run.sh`.

## Build dan runner

Build terpisah dari runner:

| Perintah | Fungsi |
|---|---|
| `bash termux/build.sh` | Build untuk Termux/Android. |
| `bash linux/build.sh` | Build untuk Linux. |
| `bash macos/build.sh` | Build untuk macOS. |
| `bash run.sh` | Shell interaktif pada terminal host. |
| `bash run-terminal.sh` | HTTP server untuk terminal browser pada `/`. |

Jalankan build satu kali sebelum menjalankan shell atau server browser. Hasil build `rioos.wasm` disimpan di root proyek.

## Penyimpanan

Mode browser menyimpan sektor MyFS pada `localStorage` browser. Mode Node.js menyimpan data di `~/.rioos-wasm32/myfs-sectors.json`; lokasi dapat diubah dengan `RIOOS_DATA_DIR`. Data browser mengikuti batas kuota dan kebijakan penyimpanan browser.

## Input browser

Fokuskan kolom perintah, ketik perintah shell, lalu tekan Enter. Tombol Clear, Copy, dan Restart tersedia pada halaman terminal. Tidak ada navigasi aplikasi, layar Home, Control Centre, atau framebuffer UI.
