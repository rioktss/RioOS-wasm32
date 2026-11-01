# Portable build notes

Build target utama port ini adalah WebAssembly 32-bit. Builder internal berada di `wasm/build.py` dan menghasilkan `rioos.wasm` pada root proyek.

Build melalui skrip platform:

```bash
bash termux/build.sh --clean
bash linux/build.sh
bash macos/build.sh
```

Untuk Windows PowerShell gunakan `windows/build.ps1`. Setelah build, jalankan `bash run.sh` untuk terminal host atau `bash run-terminal.sh` untuk terminal browser lokal.

Folder `linux/`, `termux/`, dan `macos/` masing-masing menyediakan `build.sh`, `run.sh`, dan `run-terminal.sh`. Folder `windows/` menyediakan skrip PowerShell yang setara.

Folder hosting statis harus memiliki `index.html` dan `rioos.wasm` berdampingan di root.
