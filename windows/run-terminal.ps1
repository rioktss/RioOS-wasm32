$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$Wasm = Join-Path $Root 'rioos.wasm'
if (-not (Test-Path $Wasm)) { throw 'rioos.wasm is missing. Build first with: .\windows\build.ps1' }
$Python = Get-Command python -ErrorAction SilentlyContinue
if ($Python) {
    $PythonCommand = $Python.Source
    $PythonPrefix = @()
} else {
    $Py = Get-Command py -ErrorAction SilentlyContinue
    if (-not $Py) { throw 'Python 3 is required to serve the browser terminal.' }
    $PythonCommand = $Py.Source
    $PythonPrefix = @('-3')
}
$Port = if ($env:PORT) { $env:PORT } else { '8080' }
$Url = "http://127.0.0.1:$Port/"
$ServerArgs = $PythonPrefix + @('-m', 'http.server', $Port, '--bind', '127.0.0.1', '--directory', "`"$Root`"")
$Server = Start-Process -FilePath $PythonCommand -ArgumentList $ServerArgs -PassThru -NoNewWindow
try {
    Start-Sleep -Milliseconds 800
    Write-Host "RioOS WASM32 browser terminal: $Url"
    Write-Host 'Press Ctrl+C to stop the server.'
    Start-Process $Url
    Wait-Process -Id $Server.Id
} finally {
    if ($Server -and -not $Server.HasExited) {
        Stop-Process -Id $Server.Id -Force -ErrorAction SilentlyContinue
    }
}
