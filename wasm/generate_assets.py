#!/usr/bin/env python3
"""Generate WebAssembly .incbin assembly labels for RioOS source assets."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
wasm = root / "wasm"
assets = []
for base in (root / "apps", root / "assets" / "icons", root / "assets" / "wallpapers"):
    for path in sorted(base.glob("*")):
        if path.is_file() and path.suffix.lower() in (".rpk", ".bmp"):
            rel = path.relative_to(root).as_posix()
            symbol = "rio_asset_" + re.sub(r"[^A-Za-z0-9]", "_", rel).strip("_")
            assets.append((rel, symbol))
lines = [
    '.section .rodata.rio_assets,"",@',
    '.p2align 4',
]
for rel, symbol in assets:
    lines += [f'.global {symbol}_start', f'{symbol}_start:', f'.incbin "{rel}"', f'.global {symbol}_end', f'{symbol}_end:', f'.size {symbol}_start, {symbol}_end - {symbol}_start', f'.size {symbol}_end, 0', '.p2align 4']
(wasm / "generated" / "assets.S").write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"generated {len(assets)} embedded assets")
