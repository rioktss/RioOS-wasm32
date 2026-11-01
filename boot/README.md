# Boot boundary

Target WASM32 tidak memakai boot assembly AArch64 atau linker script bare-metal. Entry point runtime browser/Node didefinisikan sebagai export C ABI di `wasm/port.cpp` (`rio_init`, `rio_finish_boot`, dan API event/framebuffer lainnya).

Direktori ini dipertahankan agar layout modul selaras dengan proyek MyKernel utama. Build WASM tidak mengompilasi file linker/assembly native.
