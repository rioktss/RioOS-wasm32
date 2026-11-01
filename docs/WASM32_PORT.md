# WASM32 port notes

## Supported

- Kernel command dispatcher dan shell teks.
- MyFS/VFS menggunakan adapter penyimpanan sektor host.
- Environment variables, command history, file descriptors, dan syscall compatibility layer.
- Process metadata, logical scheduler, dan page allocator di dalam linear memory WASM.
- Portable network-stack source dan perintah diagnostik.
- Terminal host Node.js serta terminal browser dengan MyFS persisten.

## Not provided by this target

WASM32 browser bukan mesin virtual AArch64. Target ini tidak menyediakan EL0/EL1, MMU fisik, exception vectors, GIC, hardware IRQ, VirtIO, device passthrough, native SMP, preemptive CPU context switching, atau eksekusi ELF AArch64. Process/scheduler API portable tetap dapat dipakai untuk metadata dan pengujian logis, tetapi tidak mengubah WebAssembly menjadi kernel bare-metal.

Build ini sengaja tidak menyertakan framebuffer, UI grafis, atau bundled RPK apps. Seluruh antarmuka runtime berupa terminal teks.
