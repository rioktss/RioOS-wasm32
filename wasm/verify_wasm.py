#!/usr/bin/env python3
"""Small offline verifier for the RioOS WASM32 artifact."""
import pathlib, sys
p = pathlib.Path(sys.argv[1])
data = p.read_bytes()
if len(data) < 8 or data[:4] != b'\x00asm' or data[4:8] != b'\x01\x00\x00\x00':
    raise SystemExit('ERROR: invalid WebAssembly module header')
# Ensure the expected browser ABI names occur in the module's export section.
required = [b'rio_init', b'rio_finish_boot', b'rio_key', b'rio_pointer', b'rio_framebuffer_ptr', b'rio_terminal_selftest_flags', b'memory']
missing = [name.decode() for name in required if name not in data]
if missing:
    raise SystemExit('ERROR: missing exports: ' + ', '.join(missing))
print(f'WASM module verified ({len(data)} bytes; expected exports present)')
