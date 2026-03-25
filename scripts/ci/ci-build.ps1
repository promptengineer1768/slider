param([string]$Preset)
$ErrorActionPreference = "Stop"

Write-Host ">>> Build: build-$Preset" -ForegroundColor Cyan
cmake --build --preset "build-$Preset"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
