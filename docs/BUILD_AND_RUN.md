# Build & Run — RioOS WASM32

## 1. Gambaran umum

Port ini dikompilasi menjadi `wasm/dist/rioos.wasm`. Modul yang sama dapat dijalankan oleh Node.js (terminal host) atau browser (terminal browser / framebuffer UI). Build tidak memerlukan QEMU atau bootloader.

## 2. Dependensi

| Tool | Kegunaan |
|---|---|
| Clang / Clang++ | Kompilasi C++17 freestanding ke `wasm32-unknown-unknown` |
| LLD (`wasm-ld`) | Link modul WebAssembly |
| Python 3 | Menghasilkan assembly asset dari paket RPK/BMP |
| Node.js 18+ | Host terminal dan smoke test |
| Browser modern | WebAssembly, Canvas 2D, `localStorage` untuk mode browser |

Di Termux, paket umumnya tersedia melalui:

```bash
pkg update
pkg install clang lld python nodejs
```

Di Linux/macOS gunakan package manager masing-masing. Periksa tool:

```bash
clang++ --version
wasm-ld --version
python3 --version
node --version
```

Windows memerlukan Git Bash atau WSL untuk menjalankan build Bash, serta PowerShell untuk wrapper `.ps1`.

## 3. Build

Dari root repository:

```bash
bash linux/build.sh
# atau bash termux/build.sh / bash macos/build.sh
```

PowerShell:

```powershell
.\windows\build.ps1
```

Build menyimpan artefak runtime di `wasm/dist/`. Object file sementara berada di `wasm/dist/obj/`.

Opsi:

| Opsi | Fungsi |
|---|---|
| `--help`, `-h` | Tampilkan bantuan |
| `--clean` | Hapus object file dan modul WASM lama sebelum build |
| `--verbose`, `-v` | Cetak perintah kompilasi secara detail |
| `--run` | Build, lalu masuk ke mode terminal host |

Contoh:

```bash
bash wasm/build.sh --clean
bash wasm/build.sh --verbose
bash wasm/build.sh --run
```

Compiler dan linker dapat dioverride melalui `CLANG`, `CLANGXX`, `WASM_LD`, dan `PYTHON`.

## 4. Menjalankan terminal host

```bash
bash linux/run.sh
# Termux: bash termux/run.sh
# macOS: bash macos/run.sh
# Windows: .\windows\run.ps1
```

Runner menggunakan Node.js dan `wasm/terminal.cjs`. Command yang diketik diproses oleh `rio_cli_execute()` yang masuk ke command dispatcher kernel. Backend disk lokal menggunakan JSON sektor di `~/.rioos-wasm32/myfs-sectors.json`.

Ubah direktori data dengan:

```bash
RIOOS_DATA_DIR="$HOME/rioos-data" bash linux/run.sh
```

## 5. Menjalankan terminal browser

```bash
bash linux/run-terminal.sh
```

Secara default server berjalan pada `127.0.0.1:8080`. Ganti port atau alamat bind:

```bash
PORT=9000 BIND=0.0.0.0 bash wasm/run-terminal.sh
```

Perintah terminal browser dieksekusi di WASM. Data MyFS disimpan menggunakan browser `localStorage`, dengan prefix key `rioos-wasm32-myfs-v1:`. Jangan gunakan browser private/incognito bila data harus tetap ada setelah sesi ditutup. Menghapus data situs dapat menghapus disk virtual ini.

Di Windows:

```powershell
.\windows\run-terminal.ps1
```

## 6. Menjalankan framebuffer UI

```bash
bash wasm/run-ui.sh
```

Halaman ini tersedia sebagai `/ui.html` dari server yang sama. UI menampilkan framebuffer 1024×768 di Canvas. Gunakan tap/click untuk membuka item; keyboard mendukung Arrow keys, Enter, dan Escape. UI terdiri dari lock screen, Home, Control Centre, dan tujuh aplikasi bawaan.

Runner UI browser terpisah dari terminal host Node. Server harus tetap berjalan selama halaman digunakan.

## 7. Smoke test

```bash
node wasm/test_wasm.cjs
```

Test ini memuat file WASM melalui Node, memeriksa export/import, boot dan render, navigasi UI, membuka tujuh aplikasi, menjalankan `selftest`, menguji dispatcher CLI, lalu membuat instance baru untuk memastikan data MyFS dapat dipulihkan.

Build strict menggunakan `-Wall -Wextra -Werror`. Test yang lulus hanya menjamin jalur yang tercakup; pengujian di browser nyata tetap diperlukan.

## 8. Troubleshooting

**`wasm-ld: command not found`** — instal LLD atau set `WASM_LD` ke executable `wasm-ld`.

**`clang++: unknown target wasm32-unknown-unknown`** — gunakan distribusi Clang dengan backend WebAssembly.

**`WebAssembly.instantiate` gagal** — rebuild dengan `bash wasm/build.sh`, pastikan `rioos.wasm` dan halaman HTML berasal dari folder `wasm/dist` yang sama, lalu reload browser.

**Browser menampilkan storage error** — periksa izin penyimpanan situs dan kuota `localStorage`. Smoke test Node menggunakan file storage tersendiri.

**Port 8080 dipakai** — jalankan dengan `PORT=9000`.

**Halaman tidak dapat mengambil `rioos.wasm`** — akses melalui HTTP server runner, jangan membuka `index.html` atau `ui.html` langsung sebagai `file://`.

## 9. Batas runtime

Browser tidak mengekspos hardware MMU/GIC/IRQ, AArch64 EL, VirtIO, dan raw device. Lihat [WASM32 Port](WASM32_PORT.md) untuk pemisahan fitur portable dan fitur khusus native.
