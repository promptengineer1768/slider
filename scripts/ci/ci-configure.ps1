param([string]$Preset)
$ErrorActionPreference = "Stop"

Write-Host ">>> Configure: $Preset" -ForegroundColor Cyan
cmake --preset $Preset
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
