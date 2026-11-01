<<<<<<< HEAD
# RioOS - MyKernel v1.11.0 (wasm32)

> A lightweight experimental kernel that boots directly in your browser!

### 🚀 Try it Live: https://rioktss.github.io/RioOS-wasm32/

No QEMU, no install, no compile. Just click and boot in 17 seconds.

![MyKernel](https://img.shields.io/badge/Kernel-MyKernel%20v1.11.0-blue)
![Arch](https://img.shields.io/badge/Arch-wasm32-orange)
![Platform](https://img.shields.io/badge/Platform-Browser-black)
![License](https://img.shields.io/badge/License-MIT-green)

### What is RioOS?

RioOS is an experimental kernel built on top of MyKernel, ported to WebAssembly (wasm32) to run on any modern browser. Built entirely from a phone in Bandung, Indonesia.

**System Info:**
- **OS:** RioOS
- **Kernel:** Mykernel v1.11.0
- **Architecture:** wasm32
- **Machine:** Browser / WebAssembly host
- **RAM:** 32 MiB
- **Filesystem:** MyFS (persistent)
- **License:** MIT

### Quick Start

1. Visit **https://rioktss.github.io/RioOS-wasm32/**
2. Wait for boot sequence
3. Type `help` to see available commands

### Available Commands

- `version` - Show system info
- `help` - List all commands
- `ls`, `pwd`, and more

### Source Code

```bash
git clone https://github.com/rioktss/RioOS-wasm32
=======
# RioOS — MyKernel WASM32

<p align="center">
  <strong>RioOS · MyKernel v1.13.0 · WebAssembly 32-bit</strong><br>
  Port browser dari source MyKernel, dengan terminal, MyFS, shell, framebuffer, dan aplikasi bawaan.
</p>

<p align="center">
  <a href="./docs/BUILD_AND_RUN.md">Build &amp; Run</a> ·
  <a href="./docs/ARCHITECTURE.md">Architecture</a> ·
  <a href="./docs/WASM32_PORT.md">WASM32 Port</a> ·
  <a href="./docs/LEARN.md">Learning Guide</a> ·
  <a href="./docs/ROADMAP.md">Roadmap</a>
</p>

> 🇮🇩 Made in Indonesia. Built to be studied, improved, and shared.

## Tentang proyek

RioOS WASM32 adalah port browser dari proyek MyKernel. Source bersama disinkronkan dengan baseline MyKernel **v1.13.0**. Kode berjalan sebagai modul WebAssembly di browser atau Node.js; tidak melakukan boot bare-metal dan tidak mengemulasikan mesin AArch64.

Port ini mempertahankan subsistem portable yang dapat dijalankan di sandbox WASM, sekaligus menyediakan adapter untuk fasilitas host seperti waktu, penyimpanan, terminal, dan framebuffer. Bagian yang memerlukan privilege atau perangkat QEMU tidak dipalsukan sebagai fitur native.

## Quick start

Toolchain yang dibutuhkan: Clang/Clang++, `wasm-ld` (LLD), Python 3, dan Node.js 18+. Untuk mode UI browser, gunakan browser modern dengan WebAssembly, Canvas 2D, dan `localStorage`.

### Linux

```bash
bash linux/build.sh
bash linux/run.sh             # Shell RioOS di terminal host
bash linux/run-terminal.sh    # Terminal browser
```

### Termux Android

```bash
pkg install clang lld python nodejs
bash termux/build.sh
bash termux/run.sh
bash termux/run-terminal.sh
```

### macOS

```bash
bash macos/build.sh
bash macos/run.sh
bash macos/run-terminal.sh
```

### Windows PowerShell

```powershell
.\windows\build.ps1
.\windows\run.ps1
.\windows\run-terminal.ps1
```

Windows build menggunakan Git Bash atau WSL dengan Clang, LLD, dan Python 3 terpasang. Node.js diperlukan untuk `run.ps1`; Python 3 diperlukan untuk menyajikan browser terminal.

### Mode browser framebuffer

Mode UI pixel dengan lock screen, Home, Control Centre, dan tujuh aplikasi tersedia melalui:

```bash
bash wasm/run-ui.sh
```

Script akan membuka halaman `/ui.html`. Bila server terminal sudah berjalan, halaman UI dapat diakses pada port yang sama dengan menambahkan `/ui.html` ke URL server. Pada Termux, script mencoba membuka URL menggunakan `termux-open-url`; jika tidak berhasil, buka manual di browser.

## Mode menjalankan

| Entry point | Fungsi |
|---|---|
| `*/run.sh` | Shell interaktif RioOS di terminal host melalui Node.js, memakai command dispatcher kernel yang sama. |
| `*/run-terminal.sh` | Menyajikan terminal browser di `http://127.0.0.1:8080/`. Perintah diproses oleh modul WASM, bukan simulator JavaScript. |
| `wasm/run-ui.sh` | Menyajikan framebuffer UI 1024×768 melalui Canvas; termasuk lock screen, Home, Control Centre, dan tujuh aplikasi. |

Terminal browser menyimpan sektor MyFS ke `localStorage`. Mode Node menyimpan sektor di `~/.rioos-wasm32/myfs-sectors.json`; ubah lokasi dengan `RIOOS_DATA_DIR`. Penyimpanan browser mengikuti kuota dan kebijakan penghapusan data situs dari browser.

## Fitur yang dipertahankan

- Kernel command dispatcher terbaru, shell, prompt, environment (`export`, `unset`), history, file descriptors, dan syscall compatibility layer.
- MyFS serta VFS dengan operasi file/direktori dan backend penyimpanan sektor.
- IPv4/ARP/ICMP/UDP/DNS/TCP stack source yang sama dengan baseline utama; perangkat jaringan riil tetap bergantung pada backend platform dan belum tersedia di browser.
- Process table, metadata proses, lifecycle PID, scheduler policy, logical scheduler, dan diagnostic/status command.
- Generic page allocator 4 KiB dengan pool linear-memory WASM dan statistik alokasi.
- Framebuffer renderer 1024×768, lock screen, Home, Control Centre, wallpaper lokal, serta aplikasi Calculator, Clock, Files, Launcher, Settings, Terminal, dan Wallpaper.
- Terminal browser dan host terminal yang menggunakan dispatcher kernel aktual, bukan implementasi command palsu.
- Smoke test WASM, host regression tests, test matrix, dokumentasi, dan wrapper build/run tiap platform.

## Batas platform WASM32

WASM32 browser tidak mempunyai akses ke EL1/EL0 AArch64, MMU fisik, exception vector, GIC, IRQ hardware, VirtIO, perangkat blok/jaringan QEMU, atau kemampuan untuk menjalankan AArch64 ELF. Scheduler yang tersedia mengelola metadata dan pemilihan PID secara logis; ini bukan context switch CPU. Thread native tidak dijalankan paralel. Backend jaringan tidak terhubung ke VirtIO-Net.

Fitur-fitur tersebut ditandai tidak tersedia oleh adapter, bukan diklaim aktif. Lihat [WASM32 Port](./docs/WASM32_PORT.md) untuk matriks dukungan yang lebih rinci.

## Struktur proyek

```text
mykernel-wasm32/
├── README.md
├── .gitignore
├── boot/                 # Catatan entry point WASM (tanpa boot assembly AArch64)
├── drivers/              # Framebuffer dan font browser
├── include/              # Public headers bersama + API framebuffer/UI WASM
├── kernel/               # Command dispatcher, syscall, diagnostics, history, env
├── memory/               # Generic page allocator + WASM linear-memory allocator
├── process/              # Process metadata dan scheduler policy/logis
├── fs/ net/              # MyFS/VFS dan network stack portable
├── storage/              # Area data/runtime; image lokal tidak ikut source control
├── ui/ userspace/        # Framebuffer UI dan tujuh paket aplikasi RPK
├── tools/ tests/         # Asset generator, host tests, smoke/release checks
├── wasm/                 # Platform adapter, build, browser pages, runner, tests
├── linux/ termux/ macos/ # build.sh, run.sh, run-terminal.sh
├── windows/              # build.ps1, run.ps1, run-terminal.ps1
└── docs/                 # Dokumentasi proyek dan batas port WASM32
```

## Build dan validasi

```bash
bash wasm/build.sh
node wasm/test_wasm.cjs
```

Build menggunakan `-Wall -Wextra -Werror`; test memeriksa module WebAssembly, boot, render framebuffer, navigasi UI, seluruh tujuh aplikasi, command dispatcher, self-test, dan pemulihan data MyFS setelah reload. Jalankan tes pada browser sasaran juga karena smoke test Node tidak menggantikan pengujian lintas-browser.

Perintah build menerima `--help`, `--clean`, `--verbose` / `-v`, dan `--run`.

## Lisensi

RioOS dirilis di bawah [MIT License](./docs/LICENSE).
>>>>>>> 9e1c35b (RioOS update)
