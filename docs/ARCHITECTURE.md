# Arsitektur MyKernel WASM32

## Struktur modul

| Folder | Peran |
|---|---|
| `boot/` | Catatan target WASM; tidak melakukan boot firmware atau bootloader. |
| `include/` | Header API portable kernel. |
| `kernel/` | Command dispatcher, syscall, diagnostic, history, environment, dan version. |
| `memory/` | Generic page allocator dan adapter linear memory WASM. |
| `process/` | Process metadata, scheduler, dan policy. |
| `fs/` | MyFS dan VFS. |
| `net/` | Source network stack portable. |
| `storage/` | Lokasi data dan metadata storage lokal. |
| `wasm/` | Adapter host, builder Python, terminal Node.js, dan smoke test. |
| `linux/`, `termux/`, `macos/` | Wrapper dua runner: `run.sh` dan `run-terminal.sh`. |
| `docs/`, `tests/`, `tools/` | Dokumentasi, tes, dan utilitas pengembangan. |

## Runtime

`index.html` pada root hosting memuat `rioos.wasm` dari path relatif `./rioos.wasm`. Modul mengekspos boot API dan buffer command-line. Perintah diteruskan ke `execute_command()` C++ melalui `rio_cli_execute`; JavaScript hanya menangani input/output, waktu, dan penyimpanan sektor host.

Tidak ada UI grafis, framebuffer, aplikasi RPK, atau source aplikasi UI dalam build WASM32 ini.
