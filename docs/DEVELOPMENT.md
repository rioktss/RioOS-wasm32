# Development

1. Ubah source portable pada `kernel/`, `memory/`, `process/`, `fs/`, atau `net/`.
2. Jalankan `python3 wasm/build.py --clean` untuk strict build WASM32.
3. Jalankan `node wasm/test_wasm.cjs` untuk menguji API shell dan MyFS.
4. Jalankan `python3 tests/run_host_tests.py` untuk regression tests host portable.
5. Untuk browser, jalankan `bash run-terminal.sh` dan uji `index.html` pada browser sasaran.

`rioos.wasm` dan `index.html` harus tetap berdampingan saat deploy. Tidak ada UI grafis dalam target ini.
