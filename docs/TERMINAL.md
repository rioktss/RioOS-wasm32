# Terminal RioOS WASM32

RioOS menyediakan dua jalur terminal yang menggunakan command dispatcher kernel yang sama.

| Runner | Perilaku |
|---|---|
| `linux/run.sh`, `termux/run.sh`, `macos/run.sh` | Shell interaktif di terminal host melalui Node.js. |
| `linux/run-terminal.sh`, `termux/run-terminal.sh`, `macos/run-terminal.sh` | Terminal browser di `http://127.0.0.1:8080/`. |
| `windows/run.ps1` | Shell Node.js di PowerShell. |
| `windows/run-terminal.ps1` | Terminal browser melalui Python HTTP server. |
| `wasm/run-ui.sh` | Framebuffer UI, bukan terminal teks, pada `/ui.html`. |

## Command dispatcher

Browser terminal dan host terminal meneruskan command ke fungsi export `rio_cli_execute()`. JavaScript/Node hanya menangani input, output, terminal history, dan host storage; command shell dieksekusi oleh kode C++ RioOS di WebAssembly.

Beberapa operasi yang memerlukan input password interaktif atau perangkat hardware tertentu dapat dibatasi oleh host adapter. Perintah diagnostik akan melaporkan capability sesuai target browser.

## Penyimpanan

Mode browser menyimpan MyFS per sektor di `localStorage`. Mode Node menyimpan data JSON di `~/.rioos-wasm32/myfs-sectors.json`. Hapus data situs atau file tersebut hanya bila memang ingin menghapus filesystem virtual.

## Shortcut browser

- `Enter` — kirim perintah.
- `Arrow Up` / `Arrow Down` — navigasi history terminal.
- `Ctrl+L` — clear tampilan terminal.
- `Ctrl+C` — batalkan input saat ini (tidak mematikan proses browser).

Lihat kontrol UI pada footer halaman `/ui.html` untuk navigasi lock screen dan aplikasi.
