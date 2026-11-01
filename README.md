RioOS WASM32

A WebAssembly-based port of RioOS running directly in your web browser.

<p align="center">
  <a href="https://rioktss.github.io/RioOS-wasm32/">
    <img src="https://img.shields.io/badge/Live_Demo-Visit_RioOS-blue?style=for-the-badge" alt="Live Demo">
  </a>
</p>Live Demo:
https://rioktss.github.io/RioOS-wasm32/

---

Overview

RioOS WASM32 is a browser-compatible edition of RioOS, compiled to WebAssembly (WASM32). It provides an interactive terminal environment powered by the shared RioOS kernel source.

Features

- WebAssembly kernel execution
- Interactive browser terminal
- Host terminal support
- MyFS and VFS
- Shell and command dispatcher
- Persistent filesystem storage
- Cross-platform build scripts

Build Requirements

- Clang / Clang++
- LLVM LLD ("wasm-ld")
- Python 3
- Node.js

Build & Run

Termux:

bash termux/build.sh
bash termux/run.sh

Browser terminal:

bash termux/run-terminal.sh

Directory Structure

RioOS-wasm32/
├── index.html
├── run.sh
├── run-terminal.sh
├── boot/
├── kernel/
├── memory/
├── process/
├── storage/
├── fs/
├── net/
├── include/
├── tests/
├── tools/
├── wasm/
├── linux/
├── termux/
├── macos/
└── windows/

Platform Limitations

WASM32 operates within the browser sandbox and does not provide direct access to native hardware features such as physical MMU, GIC, hardware interrupts, or native SMP.

License

See "LICENSE" (LICENSE).

---

<p align="center">
  <strong>RioOS</strong><br>
  <sub>Bringing Operating Systems to the Web.</sub>
</p>
