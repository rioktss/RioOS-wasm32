$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Bash = Get-Command bash -ErrorAction SilentlyContinue
if (-not $Bash) { throw 'Build WASM32 on Windows using Git Bash or WSL, then install clang, wasm-ld and Python 3.' }
& $Bash.Source (Join-Path $Root 'wasm/build.sh') @args
if ($LASTEXITCODE -ne 0) { throw "WASM32 build failed with exit code $LASTEXITCODE" }
