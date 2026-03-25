param([string]$Preset)
$ErrorActionPreference = "Stop"

Write-Host ">>> Test: test-$Preset" -ForegroundColor Cyan
ctest --preset "test-$Preset" --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
