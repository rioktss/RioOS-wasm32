$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Wasm = Join-Path $Root 'rioos.wasm'
if (-not (Test-Path $Wasm)) { throw 'rioos.wasm is missing. Build first with: .\windows\build.ps1' }
$Node = Get-Command node -ErrorAction SilentlyContinue
if (-not $Node) { throw 'Node.js is required. Install Node.js and add it to PATH.' }
& $Node.Source (Join-Path $Root 'wasm/terminal.cjs') @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
