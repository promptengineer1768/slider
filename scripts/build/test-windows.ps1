$ErrorActionPreference = "Stop"

$presets = @(
    "windows-msvc-debug",
    "windows-msvc-release",
    "windows-clangcl-debug",
    "windows-gcc-debug"
)

$scriptDir = Split-Path $MyInvocation.MyCommand.Path -Parent
Import-Module (Join-Path $scriptDir "..\helpers\common.ps1") -Force
Ensure-ToolsInPath

foreach ($preset in $presets) {
    $buildDir = Join-Path $PSScriptRoot "..\..\build\$preset"
    if (Test-Path $buildDir) {
        Write-Host ">>> Test: $preset" -ForegroundColor Cyan
        ctest --preset "test-$preset" --output-on-failure
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Tests failed for $preset"
            exit $LASTEXITCODE
        }
    }
}

Write-Host ">>> All tests passed" -ForegroundColor Green
