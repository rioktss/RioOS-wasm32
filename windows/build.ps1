$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Python = Get-Command python -ErrorAction SilentlyContinue
if ($Python) {
    & $Python.Source (Join-Path $Root 'wasm/build.py') @args
} else {
    $Py = Get-Command py -ErrorAction SilentlyContinue
    if (-not $Py) { throw 'Python 3 is required. Install Python 3 and add it to PATH.' }
    & $Py.Source -3 (Join-Path $Root 'wasm/build.py') @args
}
if ($LASTEXITCODE -ne 0) { throw "WASM32 build failed with exit code $LASTEXITCODE" }
