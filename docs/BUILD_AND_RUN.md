# Build dan menjalankan MyKernel WASM32

## Persyaratan

- Clang/Clang++ dengan target `wasm32-unknown-unknown`.
- `wasm-ld` dari LLVM LLD.
- Python 3.
- Node.js untuk mode terminal host.
- Python 3 untuk HTTP server terminal browser lokal.

Di Termux, pasang toolchain dengan `pkg install clang lld python nodejs`.

## Build terpisah

Build dilakukan secara eksplisit melalui skrip platform:

```bash
bash termux/build.sh   # Termux / Android
bash linux/build.sh    # Linux
bash macos/build.sh    # macOS
```

Di Windows PowerShell:

```powershell
.\windows\build.ps1
```

Gunakan `--clean` untuk build bersih, misalnya `bash termux/build.sh --clean`. Builder menghasilkan `rioos.wasm` langsung di root proyek. Builder internalnya adalah `wasm/build.py`.

## Runner

Jalankan setelah build berhasil:

```bash
bash run.sh
bash run-terminal.sh
```

`run.sh` menjalankan shell pada terminal host menggunakan Node.js. `run-terminal.sh` menjalankan HTTP server lokal agar terminal browser dapat dibuka di `/`. Keduanya tidak melakukan build otomatis; bila `rioos.wasm` belum ada, jalankan skrip build platform terlebih dahulu.

Di Windows gunakan `windows/run.ps1` untuk shell host atau `windows/run-terminal.ps1` untuk browser.

## Hosting statis

File deploy harus berada berdampingan pada root hosting:

```text
public_html/
├── index.html
└── rioos.wasm
```

`index.html` sudah berada di root repository dan memuat `./rioos.wasm`. Upload kedua file tersebut ke `public_html` atau document root layanan hosting.

## Pengujian

```bash
node wasm/test_wasm.cjs
python3 tests/run_host_tests.py
```

Builder memakai C++17, `-Wall -Wextra -Werror`, dan menghasilkan WebAssembly pada root proyek.

## Batas platform

Target browser tidak memiliki MMU/GIC/IRQ fisik, VirtIO, SMP native, atau eksekusi ELF AArch64. Build ini hanya menyediakan shell teks dan tidak menyertakan UI maupun aplikasi grafis.
