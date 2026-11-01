# Arsitektur RioOS WASM32

## 1. Model runtime

RioOS WASM32 adalah build freestanding C++17 dengan target `wasm32-unknown-unknown`. Browser atau Node.js bertindak sebagai host runtime. Tidak ada guest Linux, QEMU machine, boot ROM, atau privilege level AArch64.

Titik masuk diekspor oleh modul WASM:

- `rio_init()` — inisialisasi kernel portable, MyFS/VFS, security metadata, dan UI.
- `rio_finish_boot()` — menyelesaikan tampilan boot dan menggambar lock screen.
- `rio_key()` / `rio_pointer()` — mengirim event input ke UI.
- `rio_tick()` — memperbarui timer host/logical scheduler dan UI.
- `rio_cli_execute()` — menjalankan perintah terminal melalui command dispatcher kernel.
- `rio_framebuffer_ptr()` dan fungsi framebuffer lain — mengakses surface render.

## 2. Lapisan source

| Folder | Tanggung jawab |
|---|---|
| `kernel/` | Shell commands, diagnostics, fault accounting, FD, history, environment, syscall dispatcher, version |
| `fs/` | MyFS dan VFS |
| `net/` | IPv4, ARP, ICMP, UDP, DNS, bounded TCP client source |
| `memory/` | Generic 4 KiB page allocator dan WASM linear-memory arena |
| `process/` | Process table, context metadata, scheduler policy, logical scheduler |
| `drivers/` | Software framebuffer, font dan glyph data |
| `ui/` | Lock screen, Home, Control Centre dan tujuh aplikasi |
| `userspace/apps/` | Paket aplikasi RPK yang ditanam ke dalam modul |
| `wasm/` | Platform adapter, host storage/time/console bridge, browser pages dan runner |
| `include/` | Kontrak API yang digunakan lintas modul |

## 3. Alur boot

1. `rio_init()` membuat command environment dan menginisialisasi filesystem.
2. `rio_seed_assets()` memasukkan paket aplikasi dan wallpaper bawaan dari embedded asset.
3. Security metadata dan VFS disiapkan.
4. `fb_test_attach()` memasang array framebuffer software 1024×768.
5. `rio_finish_boot()` menggambar lock screen.
6. Browser memanggil `rio_tick()` pada event loop dan mengirim pointer/keyboard event melalui export API.

## 4. Storage

MyFS tetap menggunakan antarmuka blok sektor. Implementasi `storage_*` pada `wasm/platform.cpp` meneruskan pembacaan/penulisan ke import host. Browser page memakai `localStorage`; Node runner memakai file JSON host. Ini merupakan backend penyimpanan virtual, bukan driver VirtIO block.

## 5. Memory

`memory/wasm_memory.cpp` menyediakan heap bump terbatas dan pool page 4 KiB yang dialokasikan dari linear memory WASM. `memory/page_alloc.cpp` adalah generic allocator yang dipakai bersama source utama. Alokasi page ini bukan pemetaan halaman virtual dan tidak mengaktifkan MMU.

## 6. Process dan scheduler

`process/process.cpp`, `process/scheduler.cpp`, dan `process/sched_policy.cpp` merupakan source portable bersama. Port WASM mengompilasi process table, PID lifecycle, state dan saved-context metadata serta logical scheduler. Karena WebAssembly browser tidak memberi akses ke stack CPU kernel/privilege switching, scheduler ini tidak melakukan perpindahan eksekusi antarkonteks native.

## 7. Networking

`net/netstack.cpp` menyimpan implementasi portable stack bersama, termasuk TCP client. Browser adapter tidak menghubungkan stack ke VirtIO-Net atau raw socket API host. Karena itu keberadaan kode TCP tidak berarti koneksi internet guest telah tersedia.

## 8. Adapter host

`wasm/platform.cpp` mengimplementasikan kontrak platform yang dibutuhkan build: UART-compatible output, storage sectors, RTC, monotonic timer, security compatibility, status stubs, dan capability reporting. API hardware yang tidak dapat direalisasikan di browser mengembalikan status unavailable atau no-op yang terdokumentasi.
