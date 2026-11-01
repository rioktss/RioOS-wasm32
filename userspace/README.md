# Userspace assets

Paket aplikasi `.rpk` dan bitmap yang dipakai browser berada di `userspace/apps/` serta `userspace/assets/`. `wasm/generate_assets.py` mengubah file-file tersebut menjadi embedded WebAssembly assets.

File assembly dan linker di direktori ini berasal dari target native AArch64 dan hanya disertakan sebagai referensi source; file tersebut tidak dikompilasi oleh `wasm/build.sh`.
