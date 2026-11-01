# RioOS WASM32 Test Matrix

## Build-time WebAssembly verification

Run:

```bash
bash wasm/build.sh
python3 tests/verify_release.py
node wasm/test_wasm.cjs
```

The strict WASM build uses C++17 and `-Wall -Wextra -Werror`. The release verifier checks the module header/exports, browser pages, platform wrappers and the seven built-in application packages.

The Node integration smoke test covers:

- Exact documented host imports and required WASM exports.
- Module initialization, seeded assets, lock screen, Home and non-blank framebuffer.
- Opening and closing all seven built-in applications.
- Pixel terminal command dispatch and six WASM self-tests.
- Host CLI command dispatch, cwd updates, echo, redirection and file readback.
- Persistent MyFS sector reload into a fresh WASM instance.

## Portable host regression tests

Run:

```bash
bash tests/run_host_tests.sh
```

This compiles and executes 14 host tests for page allocator/fault counters, scheduler policy, environment, diagnostics, process lifecycle, scheduler/process integration, syscall compatibility, IPv4/ARP/UDP/DNS/TCP stack logic, MyFS, VFS, file descriptors, framebuffer, UI rendering, wallpaper/BMP decoding and UI settings.

These host tests use test doubles for storage, clock, fonts, or UI boundaries where appropriate. They validate the shared portable modules, not the browser's DOM or every browser implementation.

## Browser runtime checks still required

On a real browser, run both:

```bash
bash wasm/run-terminal.sh
bash wasm/run-ui.sh
```

Check that:
- `/` loads the browser terminal and commands execute through the kernel dispatcher.
- `/ui.html` boots to the lock screen, accepts pointer/touch and keyboard input, opens every app, and returns Home.
- MyFS files remain after reload when the browser preserves `localStorage`.
- A browser storage denial/quota error is reported without claiming the disk is persistent.

The Node test exercises the WASM module and its host bridge but does not automate real browser Canvas, DOM, mobile touch, browser quota, or cross-browser rendering.

## Platform exclusions (not test failures)

Physical AArch64 MMU, GIC/IRQ, exception levels/vectors, AArch64 ELF execution, native kernel stack switching, SMP, and QEMU VirtIO block/network/input are outside the browser WASM32 execution model. The platform adapter reports these as unavailable; a passing test must not be interpreted as support for them.
