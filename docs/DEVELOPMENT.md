# Development Guide — RioOS WASM32

## Workflow

1. Ubah source di modul yang sesuai.
2. Jalankan `bash wasm/build.sh` agar semua source target dikompilasi dengan warning sebagai error.
3. Jalankan `node wasm/test_wasm.cjs`.
4. Bila perubahan menyentuh browser page, uji juga terminal `/` dan framebuffer `/ui.html` pada browser nyata.
5. Perbarui `docs/WASM32_PORT.md` dan `tests/TEST_MATRIX.md` bila status fitur atau cakupan tes berubah.

## Source sharing

Source portable seperti command dispatcher, MyFS/VFS, network stack, process metadata, scheduler policy, dan page allocator disinkronkan dari baseline MyKernel utama. Jangan menyalin implementasi hardware AArch64 ke build WASM secara membabi buta. Gunakan adapter `wasm/platform.cpp` untuk kontrak platform dan dokumentasikan jika capability tidak tersedia.

`wasm/build.sh` memiliki source list eksplisit agar file AArch64 yang tidak kompatibel tidak ikut di-link. Penambahan modul baru harus menambahkan compile step yang relevan dan tes untuk perilakunya.

## Aturan kompatibilitas

- Jangan menghapus fitur lama hanya untuk menyelesaikan build baru.
- Pertahankan export API yang telah dipakai browser dan Node runner.
- Jangan mengubah format sektor MyFS tanpa migrasi dan regression test.
- Jangan menyatakan hardware capability aktif jika adapter hanya no-op atau stub.
- Gunakan `-Wall -Wextra -Werror` dan perbaiki warning, bukan menyembunyikannya.
- Hindari operasi blocking pada event loop browser.

## Debugging

- Build: `bash wasm/build.sh --verbose`
- Smoke test: `node wasm/test_wasm.cjs`
- Browser: buka Developer Tools dan periksa error fetch, instantiate, Canvas, serta localStorage quota.
- Host terminal: jalankan `node wasm/terminal.cjs` setelah build.

## Apa yang bukan diuji

Smoke test tidak menjalankan seluruh perintah dengan backend hardware, tidak menguji semua browser/device, dan tidak dapat membuktikan ketiadaan seluruh bug. Jaringan guest, MMU, GIC/IRQ, native ELF, SMP dan native context switch memang tidak menjadi target runtime WASM32.
