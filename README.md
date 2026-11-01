RioOS WASM32

Run RioOS directly in your web browser using WebAssembly.

""Live Demo" (https://img.shields.io/badge/Live%20Demo-RioOS-orange?style=for-the-badge)" (https://rioktss.github.io/RioOS-wasm32/)

🌐 Live Demo: https://rioktss.github.io/RioOS-wasm32/

Features

- WASM32 kernel port.
- Interactive browser terminal.
- Host-terminal support.
- MyFS and VFS.
- Shell and command dispatcher.
- Persistent browser storage.
- Linux, Termux, macOS, and Windows build scripts.

Build

Requirements: Clang, LLD, Python 3, and Node.js.

Termux / Linux:

bash termux/build.sh
bash termux/run.sh

Browser terminal:

bash termux/run-terminal.sh

Project Structure

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

Platform

RioOS WASM32 runs in WebAssembly and uses browser-compatible platform interfaces. Native hardware features such as physical MMU, GIC, and native SMP are not available in the browser.

License

See "LICENSE" (LICENSE).

---

RioOS — An Operating System on the Web.