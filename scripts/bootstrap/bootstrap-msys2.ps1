# scripts/bootstrap/bootstrap-msys2.ps1
# Idempotent bootstrap for MSYS2 environment (MinGW-w64/UCRT64/GCC).
# Does NOT modify the current session's PATH - MSYS2 bash manages its own PATH.

$ErrorActionPreference = "Stop"

$msysRoot = "C:\msys64"
if (-not (Test-Path $msysRoot)) {
    Write-Host "MSYS2 not found. Installing via winget..." -ForegroundColor Yellow
    winget install MSYS2.MSYS2 -e --silent --accept-source-agreements --accept-package-agreements
    if (-not (Test-Path $msysRoot)) {
        Write-Error "MSYS2 installation failed."
        exit 1
    }
    Write-Host "MSYS2 installed at $msysRoot" -ForegroundColor Green
}

$msysBash = Join-Path $msysRoot "usr\bin\bash.exe"
if (-not (Test-Path $msysBash)) {
    Write-Error "MSYS2 Bash not found at $msysBash."
    exit 1
}

Write-Host ">>> Checking MSYS2/UCRT64 environment..." -ForegroundColor Cyan

# Update system packages
Write-Host "Updating MSYS2 system packages..." -ForegroundColor Yellow
& $msysBash -lc "pacman -Syu --noconfirm"
& $msysBash -lc "pacman -Su --noconfirm"

# Required UCRT64 packages
$packages = @(
    "mingw-w64-ucrt-x86_64-gcc",
    "mingw-w64-ucrt-x86_64-cmake",
    "mingw-w64-ucrt-x86_64-ninja",
    "mingw-w64-ucrt-x86_64-wxwidgets3.2-msw",
    "mingw-w64-ucrt-x86_64-gtest"
)

$packageList = $packages -join " "

Write-Host "Installing packages: $packageList" -ForegroundColor Yellow
& $msysBash -lc "pacman -S --needed --noconfirm $packageList"

if ($LASTEXITCODE -ne 0) {
    Write-Error "Pacman failed to install one or more packages."
    exit 1
}

# Verify versions
Write-Host ">>> Verifying installed versions:" -ForegroundColor Cyan

$checks = @(
    @{ name = "GCC"; cmd = "/ucrt64/bin/g++ --version | head -n 1" },
    @{ name = "CMake"; cmd = "/ucrt64/bin/cmake --version | head -n 1" },
    @{ name = "Ninja"; cmd = "/ucrt64/bin/ninja --version" }
)

foreach ($check in $checks) {
    $output = & $msysBash -lc "$($check.cmd)"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "$($check.name) not found in UCRT64 environment."
        exit 1
    }
    Write-Host "  $($check.name): $output" -ForegroundColor Green
}

Write-Host ">>> MSYS2 UCRT64 environment ready." -ForegroundColor Green
