$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Python = Get-Command python -ErrorAction SilentlyContinue
if (-not $Python) { throw 'Python 3 is required to serve the browser terminal.' }
$Wasm = Join-Path $Root 'wasm/dist/rioos.wasm'
if (-not (Test-Path $Wasm) -or $env:RIOOS_REBUILD -eq '1') { & (Join-Path $PSScriptRoot 'build.ps1') }
Copy-Item (Join-Path $Root 'wasm/index.html') (Join-Path $Root 'wasm/dist/index.html') -Force
Copy-Item (Join-Path $Root 'wasm/ui.html') (Join-Path $Root 'wasm/dist/ui.html') -Force
$Port = if ($env:PORT) { $env:PORT } else { '8080' }
Write-Host "RioOS WASM32 browser terminal: http://127.0.0.1:$Port/"
Write-Host "Framebuffer UI: http://127.0.0.1:$Port/ui.html"
& $Python.Source -m http.server $Port --bind 127.0.0.1 --directory (Join-Path $Root 'wasm/dist')
