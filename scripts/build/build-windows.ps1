param(
    [Parameter(Mandatory=$true)]
    [ValidateSet("msvc", "clang", "gcc")]
    [string]$Compiler,

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$presetMap = @{
    "msvc-Debug" = "windows-msvc-debug"
    "msvc-Release" = "windows-msvc-release"
    "clang-Debug" = "windows-clangcl-debug"
    "gcc-Debug" = "windows-gcc-debug"
}

$presetKey = "$Compiler-$Config"
$preset = $presetMap[$presetKey]
if (-not $preset) {
    Write-Error "No preset for compiler '$Compiler' and config '$Config'."
    exit 1
}

Write-Host ">>> Build: $preset" -ForegroundColor Cyan

function Find-ToolDir {
    param([string]$ExeName, [string]$KnownDir, [string]$GlobPattern)

    if ($KnownDir -and (Test-Path (Join-Path $KnownDir $ExeName))) {
        return $KnownDir
    }
    if ($GlobPattern) {
        $found = Get-ChildItem $GlobPattern -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) { return $found.DirectoryName }
    }
    return $null
}

function Assert-PathExists {
    param([string]$PathValue, [string]$Message)
    if (-not (Test-Path $PathValue)) {
        Write-Error $Message
        exit 1
    }
}

$cmakeDir = Find-ToolDir -ExeName "cmake.exe" -KnownDir "${env:ProgramFiles}\CMake\bin" -GlobPattern "${env:LOCALAPPDATA}\Microsoft\WinGet\Packages\*\cmake.exe"
$ninjaDir = Find-ToolDir -ExeName "ninja.exe" -KnownDir "${env:ProgramFiles}\Ninja" -GlobPattern "${env:LOCALAPPDATA}\Microsoft\WinGet\Packages\*\ninja.exe"
$gitDir = Find-ToolDir -ExeName "git.exe" -KnownDir "${env:ProgramFiles}\Git\cmd" -GlobPattern "${env:LOCALAPPDATA}\Microsoft\WinGet\Packages\*\git.exe"
$nsisDir = Find-ToolDir -ExeName "makensis.exe" -KnownDir "${env:ProgramFiles(x86)}\NSIS" -GlobPattern "${env:LOCALAPPDATA}\Microsoft\WinGet\Packages\*\makensis.exe"

if ($Compiler -eq "msvc" -or $Compiler -eq "clang") {
    # Build-only path: expects compilers and tools to already be installed.
    if (-not $cmakeDir) {
        Write-Error "cmake.exe not found. Install CMake >= 3.28 before running this script."
        exit 1
    }
    if (-not $ninjaDir) {
        Write-Error "ninja.exe not found. Install Ninja >= 1.11 before running this script."
        exit 1
    }
    if (-not $gitDir) {
        Write-Error "git.exe not found. Install Git >= 2.43 before running this script."
        exit 1
    }

    $vcvarsPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
    Assert-PathExists -PathValue $vcvarsPath -Message "vcvarsall.bat not found at $vcvarsPath"

    $vcpkgRoot = Join-Path $env:USERPROFILE "vcpkg"
    Assert-PathExists -PathValue $vcpkgRoot -Message "vcpkg not found at $vcpkgRoot. Bootstrap vcpkg first."

    # Controlled PATH only for the build subprocess.
    $ourDirs = @($cmakeDir, $ninjaDir, $gitDir, $nsisDir) | Where-Object { $_ }
    $ourPathPrefix = ($ourDirs -join ';')

    $tempBat = [System.IO.Path]::ChangeExtension([System.IO.Path]::GetTempFileName(), ".bat")
    $batContent = @"
@echo off
call "$vcvarsPath" x64
set PATH=$ourPathPrefix;%PATH%
set VCPKG_INSTALLATION_ROOT=$vcpkgRoot
cmake --preset $preset
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
cmake --build --preset build-$preset
"@
    Set-Content -Path $tempBat -Value $batContent -Encoding Ascii

    try {
        Write-Host ">>> Building with controlled PATH..." -ForegroundColor Yellow
        cmd /c "`"$tempBat`""
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Build failed"
            exit $LASTEXITCODE
        }
    }
    finally {
        if (Test-Path $tempBat) {
            Remove-Item -Path $tempBat -Force -ErrorAction SilentlyContinue
        }
    }
}
elseif ($Compiler -eq "gcc") {
    # Build-only path for GCC on Windows via MSYS2 UCRT64.
    # If WSL2 is available, use bootstrap-wsl2-toolchains.ps1 separately to
    # provision GNU/Clang toolchains and related dependencies.
    $msysRoot = "C:\msys64"
    $msysBash = Join-Path $msysRoot "usr\bin\bash.exe"
    Assert-PathExists -PathValue $msysBash -Message "MSYS2 Bash not found at $msysBash. Install MSYS2 first."

    # Preflight required UCRT64 tools.
    & $msysBash -lc "command -v /ucrt64/bin/cmake >/dev/null && command -v /ucrt64/bin/ninja >/dev/null && command -v /ucrt64/bin/g++ >/dev/null"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Missing UCRT64 tools (cmake/ninja/g++). Install required MSYS2 UCRT64 packages first."
        exit $LASTEXITCODE
    }

    $workDir = $PWD.Path.Replace('\\', '/')
    Write-Host ">>> Configuring $preset..." -ForegroundColor Yellow
    & $msysBash -lc "cd '$workDir' && cmake --preset $preset"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    Write-Host ">>> Building $preset..." -ForegroundColor Yellow
    & $msysBash -lc "cd '$workDir' && cmake --build --preset build-$preset"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host ">>> Build complete: $preset" -ForegroundColor Green
