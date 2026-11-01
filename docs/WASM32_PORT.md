# WASM32 Port — Matriks Fitur

Dokumen ini membedakan source portable yang benar-benar dikompilasi dengan fasilitas hardware native yang tidak tersedia dalam browser. Label **didukung** tidak berarti identik dengan implementasi bare-metal.

| Subsystem | Status WASM32 | Implementasi / batas |
|---|---|---|
| Shell dan command dispatcher | Didukung | Source dispatcher v1.13.0 dipakai oleh terminal browser dan Node host. |
| Environment dan history | Didukung | In-memory shell state. |
| File descriptor dan syscall compatibility | Didukung dengan adapter | API portable dibangun; syscall tidak menjalankan instruksi privilege AArch64. |
| MyFS dan VFS | Didukung | Sector backend browser `localStorage` atau Node JSON file. |
| Persistent storage | Didukung dengan batas host | Mengikuti quota, izin dan lifecycle data browser. |
| IPv4 / ARP / ICMP / UDP / DNS / TCP source | Source dikompilasi | Belum dipasang ke perangkat NIC browser/VirtIO; jangan menganggap `ping`, DNS atau HTTP guest dapat akses internet. |
| Process table / PID / lifecycle | Didukung | Metadata proses dan state; tidak menjalankan binary AArch64. |
| Scheduler policy / logical scheduler | Didukung | Memilih PID secara logis, bukan context switch CPU. |
| Saved process context metadata | Didukung sebagai data | Struktur register disimpan sebagai metadata; tidak direstore ke CPU. |
| Generic page allocator | Didukung | Page 4 KiB dari pool linear memory; bukan MMU atau virtual memory. |
| Heap WASM | Didukung | Arena terbatas 2 MiB pada implementasi saat ini. |
| Framebuffer software 1024×768 | Didukung | Ditransfer ke browser Canvas pada `/ui.html`. |
| Lock screen, Home, Control Centre | Didukung | Input mouse/touch dan keyboard. |
| Tujuh aplikasi RPK | Didukung | Calculator, Clock, Files, Launcher, Settings, Terminal, Wallpaper. |
| Browser terminal | Didukung | Command dispatcher WASM aktual, persistence melalui `localStorage`. |
| Node host terminal | Didukung | Command dispatcher WASM aktual, persistence melalui JSON host. |
| Physical MMU / TTBR / page table | Tidak tersedia | Browser WASM tidak memberi kontrol MMU fisik. |
| AArch64 exception vector / GIC / hardware IRQ | Tidak tersedia | Timer berasal dari host monotonic clock/event loop. |
| Native AArch64 ELF loader/execution | Tidak tersedia | Tidak dapat mengeksekusi ELF AArch64 sebagai proses WASM. |
| Native kernel threads / CPU context switch | Tidak tersedia | Browser event loop single-threaded dalam port saat ini. |
| SMP / multicore guest | Tidak tersedia | Tidak ada startup CPU guest atau scheduler multicore. |
| VirtIO block / net / input | Tidak tersedia | Diganti storage/input browser adapter, bukan emulasi device bus. |

## Catatan integrasi source

Folder dan modul bersama mengikuti layout MyKernel utama: `boot`, `drivers`, `include`, `kernel`, `memory`, `process`, `fs`, `net`, `storage`, `tests`, `tools`, dan `userspace`. Adapter browser serta build runner berada di `wasm/`; wrapper platform berada di `linux/`, `termux/`, `macos/`, dan `windows/`.

File assembly/linker AArch64 yang masih ada di `boot/` atau `userspace/` hanya disertakan sebagai referensi source proyek utama dan **tidak** masuk ke link WASM. Build list yang eksplisit di `wasm/build.sh` adalah definisi source yang benar-benar dikompilasi.

## Hal yang tidak boleh diasumsikan

- Keberadaan API di header bukan bukti bahwa hardware tersedia pada browser.
- Build berhasil bukan bukti bahwa jaringan guest, MMU, interrupt atau native thread berjalan.
- Host-side Node smoke test bukan pengujian seluruh browser dan seluruh device.
- Page allocator bukan virtual memory; process context bukan register CPU yang aktif.
