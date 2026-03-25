# scripts/bootstrap/bootstrap-wsl2-toolchains.ps1
# Idempotent bootstrap for WSL2 GNU/Clang toolchains and build dependencies.
# This script installs dependencies in WSL2 only; it does not run a project build.

[CmdletBinding()]
param(
    [ValidateSet("gcc", "clang", "both")]
    [string]$Compiler = "both",

    [string]$Distro
)

$ErrorActionPreference = "Stop"

function Get-Wsl2Distro {
    param([string]$Preferred)

    if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
        return $null
    }

    $list = & wsl.exe -l -v 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $list) {
        return $null
    }

    $wsl2Distros = @()
    foreach ($line in $list) {
        if ($line -match "VERSION" -or [string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        $normalized = ($line -replace '^\s*\*\s*', '')
        $parts = $normalized -split '\s+'
        if ($parts.Length -lt 3) {
            continue
        }

        $version = $parts[$parts.Length - 1]
        if ($version -ne "2") {
            continue
        }

        $name = $parts[0]
        if (-not [string]::IsNullOrWhiteSpace($name)) {
            $wsl2Distros += $name
        }
    }

    if (-not $wsl2Distros) {
        return $null
    }

    if ($Preferred) {
        if ($wsl2Distros -contains $Preferred) {
            return $Preferred
        }
        throw "Requested WSL distro '$Preferred' is not running on WSL2."
    }

    return $wsl2Distros[0]
}

$distroName = Get-Wsl2Distro -Preferred $Distro
if (-not $distroName) {
    Write-Error "No WSL2 distro detected. Install WSL2 and a distro first."
    exit 1
}

Write-Host ">>> Using WSL2 distro: $distroName" -ForegroundColor Cyan

$commonPackages = @(
    "ca-certificates",
    "git",
    "cmake",
    "ninja-build",
    "pkg-config",
    "build-essential",
    "mingw-w64",
    "gcc-mingw-w64-x86-64",
    "g++-mingw-w64-x86-64",
    "binutils-mingw-w64-x86-64"
)

$compilerPackages = @()
if ($Compiler -eq "gcc" -or $Compiler -eq "both") {
    $compilerPackages += @("gcc", "g++")
}
if ($Compiler -eq "clang" -or $Compiler -eq "both") {
    $compilerPackages += @("clang", "lld")
}

$packageList = ($commonPackages + $compilerPackages | Select-Object -Unique) -join " "

$installCommand = @"
set -euo pipefail
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y $packageList
"@

Write-Host ">>> Installing WSL2 packages..." -ForegroundColor Yellow
& wsl.exe -d $distroName -u root -- bash -lc $installCommand
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to install required packages in WSL2 distro '$distroName'."
    exit $LASTEXITCODE
}

$checks = @(
    "cmake --version | head -n 1",
    "ninja --version",
    "x86_64-w64-mingw32-g++ --version | head -n 1"
)
if ($Compiler -eq "clang" -or $Compiler -eq "both") {
    $checks += "clang --version | head -n 1"
}
if ($Compiler -eq "gcc" -or $Compiler -eq "both") {
    $checks += "g++ --version | head -n 1"
}

Write-Host ">>> Verifying WSL2 toolchain versions..." -ForegroundColor Cyan
foreach ($check in $checks) {
    & wsl.exe -d $distroName -- bash -lc $check
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Tool verification failed for command: $check"
        exit $LASTEXITCODE
    }
}

Write-Host ">>> WSL2 toolchain bootstrap complete." -ForegroundColor Green
