# MyKernel WASM32 — terminal-only edition

MyKernel WASM32 menjalankan command dispatcher dan subsistem portable RioOS di WebAssembly 32-bit. Edisi ini sengaja **tanpa UI grafis, tanpa framebuffer, dan tanpa paket aplikasi RPK**. Antarmuka yang tersedia hanya shell terminal.

## Persyaratan

- LLVM/Clang (`clang++`) dengan target `wasm32-unknown-unknown`.
- LLVM LLD (`wasm-ld`).
- Python 3.
- Node.js untuk menjalankan shell host.
- Python 3 juga dipakai untuk menyajikan terminal browser secara lokal.

Termux:

```bash
pkg install clang lld python nodejs
```

## Build

Build dipisahkan dari proses menjalankan program. Pilih skrip sesuai platform:

```bash
# Termux / Android
bash termux/build.sh

# Linux
bash linux/build.sh

# macOS
bash macos/build.sh

# Windows PowerShell (dengan Python 3, LLVM/LLD tersedia di PATH)
.\windows\build.ps1
```

Builder menghasilkan `rioos.wasm` langsung di root proyek. Untuk build bersih, tambahkan `--clean`:

```bash
bash termux/build.sh --clean
```

Alternatifnya, builder internal dapat dipanggil langsung dengan `python3 wasm/build.py`.

## Jalankan

Build dahulu, lalu jalankan salah satu mode:

```bash
# Shell interaktif di terminal host (Node.js)
bash run.sh

# Terminal browser lokal (HTTP server)
bash run-terminal.sh
```

Entry point per-platform juga tersedia: `bash termux/run.sh`, `bash termux/run-terminal.sh`, dan padanannya di folder `linux/` serta `macos/`. Di Windows gunakan `windows/run.ps1` atau `windows/run-terminal.ps1`.

## Hosting statis

`index.html` dan `rioos.wasm` berada langsung di root proyek. Upload keduanya ke root hosting (misalnya `public_html/`):

```text
public_html/
├── index.html
└── rioos.wasm
```

Jangan pindahkan `index.html` ke subfolder tanpa menyesuaikan path modul. Browser mengambil WebAssembly dengan path relatif `./rioos.wasm`.

## Entry point dan builder

| File | Fungsi |
|---|---|
| `linux/build.sh`, `termux/build.sh`, `macos/build.sh` | Build WASM32 pada platform terkait. |
| `windows/build.ps1` | Build WASM32 dari PowerShell. |
| `run.sh` | Menjalankan shell RioOS pada terminal host melalui Node.js. |
| `run-terminal.sh` | Menyajikan terminal browser melalui HTTP lokal. |
| `index.html` | Halaman terminal browser; berada di root hosting. |
| `rioos.wasm` | Hasil kompilasi WebAssembly. |
| `wasm/build.py` | Builder internal C++17. |
| `wasm/terminal.cjs` | Host terminal Node.js dengan penyimpanan MyFS persisten. |
| `wasm/test_wasm.cjs` | Smoke test runtime WASM. |

## Fitur terminal

- Shell dan command dispatcher C++ yang sama dengan source MyKernel.
- MyFS/VFS dan penyimpanan sektor persisten.
- Environment, command history, file descriptors, syscall compatibility layer.
- Process metadata, logical scheduler, dan page allocator pada linear memory WASM.
- Source network stack portable; akses jaringan browser tidak otomatis terhubung ke perangkat jaringan.
- Perintah diagnostik untuk menunjukkan batas fitur platform.

## Batas WASM32

Browser WASM tidak menyediakan MMU fisik AArch64, EL0/EL1, GIC, IRQ hardware, VirtIO, akses perangkat mentah, SMP native, atau eksekusi ELF AArch64. Scheduler yang ada mengelola metadata proses dan bukan context switch CPU. Build ini tidak menyertakan UI grafis atau aplikasi RPK.

## Pengujian

```bash
node wasm/test_wasm.cjs
python3 tests/run_host_tests.py
```

Pengujian Node menguji ABI WASM, command dispatcher, operasi shell, dan persistensi MyFS. Smoke test Node bukan pengganti pengujian pada browser hosting sebenarnya.

## Lisensi

MIT — lihat [`docs/LICENSE`](docs/LICENSE).
