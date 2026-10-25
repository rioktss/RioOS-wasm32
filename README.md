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