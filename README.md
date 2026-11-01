RioOS WASM32

A WebAssembly port of RioOS that runs directly in your web browser.

<p align="center">
  <a href="https://rioktss.github.io/RioOS-wasm32/">
    <strong>🌐 Live Demo</strong>
  </a>
</p>---

Overview

RioOS WASM32 brings the RioOS kernel to the web using WebAssembly (WASM32), providing an interactive terminal environment without requiring a native installation.

Features

- WebAssembly kernel execution
- Interactive browser terminal
- Host terminal support
- MyFS and VFS
- Shell and command system
- Persistent storage
- Cross-platform build scripts

Build

Requirements: Clang, LLD, Python 3, Node.js.

# Build
bash termux/build.sh

# Run
bash termux/run.sh

# Browser terminal
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

Platform Limitations

WASM32 runs inside a browser sandbox. Native hardware features such as physical MMU, GIC, hardware interrupts, and native SMP are not directly available.

License

See "LICENSE" (LICENSE).

---

<p align="center">
  <strong>RioOS</strong><br>
  <sub>An Operating System on the Web.</sub>
</p>
