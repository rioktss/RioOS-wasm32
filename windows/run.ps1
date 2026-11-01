$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Wasm = Join-Path $Root 'wasm/dist/rioos.wasm'
if (-not (Test-Path $Wasm) -or $env:RIOOS_REBUILD -eq '1') { & (Join-Path $PSScriptRoot 'build.ps1') }
$Node = Get-Command node -ErrorAction SilentlyContinue
if (-not $Node) { throw 'Node.js is required.' }
& $Node.Source (Join-Path $Root 'wasm/terminal.cjs') @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
