# scripts/bootstrap/bootstrap-windows.ps1
# Phase 1 bootstrap for Windows development/build environment.
# Default behavior installs all subphases; use flags to skip specific groups.

[CmdletBinding()]
param(
    [switch]$SkipCompilers,
    [switch]$SkipFormatting,
    [switch]$SkipLinting,
    [switch]$SkipTools,
    [switch]$SkipInstallerTools,
    [switch]$MsvcOnly,
    [switch]$ForceMsys2
)

$ErrorActionPreference = "Stop"

function Write-Phase {
    param([string]$Name)
    Write-Host ""
    Write-Host "=== $Name ===" -ForegroundColor Cyan
}

function Test-Tool {
    param([string]$Name, [string]$KnownPath)
    if ($KnownPath -and (Test-Path $KnownPath)) { return $true }
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Ensure-Tool {
    param([string]$ToolName, [string]$WingetId, [string]$KnownPath)

    if (Test-Tool -Name $ToolName -KnownPath $KnownPath) {
        if ($KnownPath -and (Test-Path $KnownPath)) {
            Write-Host "Found $ToolName at $KnownPath" -ForegroundColor Green
        }
        else {
            $cmd = Get-Command $ToolName -ErrorAction SilentlyContinue
            Write-Host "Found $ToolName at $($cmd.Source)" -ForegroundColor Green
        }
        return
    }

    Write-Host "$ToolName not found. Installing via winget..." -ForegroundColor Yellow
    winget install $WingetId -e --silent --accept-source-agreements --accept-package-agreements
    Start-Sleep -Seconds 2

    if (-not (Test-Tool -Name $ToolName -KnownPath $KnownPath)) {
        Write-Warning "$ToolName may require a new terminal session to be accessible."
    }
    else {
        Write-Host "$ToolName installed successfully." -ForegroundColor Green
    }
}

function Ensure-VSBuildTools {
    $buildToolsPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools"
    $vcvarsPath = Join-Path $buildToolsPath "VC\Auxiliary\Build\vcvarsall.bat"

    if (Test-Path $vcvarsPath) {
        Write-Host "Found VS Build Tools at $buildToolsPath" -ForegroundColor Green
        return
    }

    Write-Host "VS Build Tools not found. Downloading installer..." -ForegroundColor Yellow
    $vsTempDir = "$env:TEMP\vs_buildtools"
    if (-not (Test-Path $vsTempDir)) {
        New-Item -ItemType Directory -Path $vsTempDir | Out-Null
    }

    $vsExe = Join-Path $vsTempDir "vs_buildtools.exe"
    Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_buildtools.exe" -OutFile $vsExe

    Write-Host "Installing VS Build Tools (x64 workload)..." -ForegroundColor Yellow
    $installArgs = "--quiet", "--wait", "--norestart", "--nocache",
        "--add", "Microsoft.VisualStudio.Workload.VCTools",
        "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
        "--add", "Microsoft.VisualStudio.Component.Windows11SDK.22621",
        "--includeRecommended"

    $process = Start-Process -FilePath $vsExe -ArgumentList $installArgs -Wait -PassThru -NoNewWindow
    if ($process.ExitCode -ne 0) {
        Write-Error "VS Build Tools installation failed with exit code $($process.ExitCode)"
        exit $process.ExitCode
    }

    if (-not (Test-Path $vcvarsPath)) {
        Write-Error "VS Build Tools installation completed but vcvarsall.bat was not found at $vcvarsPath"
        exit 1
    }

    Write-Host "VS Build Tools installed successfully" -ForegroundColor Green
}

function Test-Wsl2Available {
    if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
        return $false
    }

    $list = & wsl.exe -l -v 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $list) {
        return $false
    }

    foreach ($line in $list) {
        if ($line -match "VERSION" -or [string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        if ($line -match "\s2\s*$") {
            return $true
        }
    }

    return $false
}

function Ensure-Vcpkg {
    $vcpkgRoot = Join-Path $env:USERPROFILE "vcpkg"
    if (Test-Path $vcpkgRoot) {
        Write-Host "Found vcpkg at $vcpkgRoot" -ForegroundColor Green
        return
    }

    Write-Host "vcpkg not found. Cloning into $vcpkgRoot..." -ForegroundColor Yellow
    git clone https://github.com/microsoft/vcpkg.git $vcpkgRoot
    Push-Location $vcpkgRoot
    .\bootstrap-vcpkg.bat
    Pop-Location
    Write-Host "vcpkg installed at $vcpkgRoot" -ForegroundColor Green
}

Write-Phase "Phase 1.1: Core Tools"
if (-not $SkipTools) {
    Ensure-Tool -ToolName "cmake" -WingetId "Kitware.CMake" -KnownPath "${env:ProgramFiles}\CMake\bin\cmake.exe"
    Ensure-Tool -ToolName "ninja" -WingetId "Ninja-build.Ninja" -KnownPath "${env:ProgramFiles}\Ninja\ninja.exe"
    Ensure-Tool -ToolName "git" -WingetId "Git.Git" -KnownPath "${env:ProgramFiles}\Git\cmd\git.exe"
    Ensure-Vcpkg
}
else {
    Write-Host "Skipping core tools phase." -ForegroundColor DarkYellow
}

Write-Phase "Phase 1.2: Compilers"
if (-not $SkipCompilers) {
    Ensure-VSBuildTools
    if ($MsvcOnly) {
        Write-Host "MsvcOnly enabled. Skipping GCC/Clang compiler bootstraps." -ForegroundColor Yellow
    }
    else {
        Ensure-Tool -ToolName "clang-cl" -WingetId "LLVM.LLVM" -KnownPath "${env:ProgramFiles}\LLVM\bin\clang-cl.exe"

        if ((Test-Wsl2Available) -and (-not $ForceMsys2)) {
            Write-Host "WSL2 detected. Bootstrapping GCC/Clang toolchains in WSL2 (preferred)." -ForegroundColor Yellow
            & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "bootstrap-wsl2-toolchains.ps1") -Compiler both
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
        else {
            if ($ForceMsys2) {
                Write-Host "ForceMsys2 enabled. Using MSYS2/UCRT64 bootstrap." -ForegroundColor Yellow
            }
            else {
                Write-Host "WSL2 not detected. Using MSYS2/UCRT64 bootstrap." -ForegroundColor Yellow
            }
            & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "bootstrap-msys2.ps1")
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        }
    }
}
else {
    Write-Host "Skipping compilers phase." -ForegroundColor DarkYellow
}

Write-Phase "Phase 1.3: Formatting"
if (-not $SkipFormatting) {
    Ensure-Tool -ToolName "clang-format" -WingetId "LLVM.LLVM" -KnownPath "${env:ProgramFiles}\LLVM\bin\clang-format.exe"
}
else {
    Write-Host "Skipping formatting phase." -ForegroundColor DarkYellow
}

Write-Phase "Phase 1.4: Linting"
if (-not $SkipLinting) {
    Ensure-Tool -ToolName "clang-tidy" -WingetId "LLVM.LLVM" -KnownPath "${env:ProgramFiles}\LLVM\bin\clang-tidy.exe"
}
else {
    Write-Host "Skipping linting phase." -ForegroundColor DarkYellow
}

Write-Phase "Phase 1.5: Installer Tools"
if (-not $SkipInstallerTools) {
    Ensure-Tool -ToolName "magick" -WingetId "ImageMagick.ImageMagick" -KnownPath "${env:ProgramFiles}\ImageMagick-7.1.2-Q16-HDRI\magick.exe"
    Ensure-Tool -ToolName "ISCC.exe" -WingetId "JRSoftware.InnoSetup" -KnownPath "${env:LOCALAPPDATA}\Programs\Inno Setup 6\ISCC.exe"
    Ensure-Tool -ToolName "candle.exe" -WingetId "WiXToolset.WiXToolset" -KnownPath "${env:ProgramFiles(x86)}\WiX Toolset v3.14\bin\candle.exe"
}
else {
    Write-Host "Skipping installer tools phase." -ForegroundColor DarkYellow
}

Write-Host ""
Write-Host ">>> Phase 1 bootstrap complete." -ForegroundColor Green
