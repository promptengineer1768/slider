[CmdletBinding()]
param(
    # Skip CPack ZIP output generation.
    [switch]$SkipZip,

    # Skip CPack MSI (WiX) output generation.
    [switch]$SkipMsi,

    # Skip Inno Setup installer generation.
    [switch]$SkipInstaller
)

$ErrorActionPreference = "Stop"

# Resolve repository root from scripts/build/.
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$BuildDir = Join-Path $ProjectRoot "build\windows-msvc-release"
$BuildBinDir = Join-Path $BuildDir "bin"
$DistDir = Join-Path $ProjectRoot "dist"
$WindowsDistDir = Join-Path $DistDir "windows"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "=== $Message ===" -ForegroundColor Cyan
}

function Test-Command {
    param([string]$Name)
    return $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Get-ImageMagickPath {
    # Check common install locations first, then PATH fallback.
    $candidates = @(
        "${env:ProgramFiles}\ImageMagick-7.1.2-Q16\magick.exe",
        "${env:ProgramFiles}\ImageMagick-7.1.2-Q16-HDRI\magick.exe",
        "${env:ProgramFiles}\ImageMagick-7.1.1-Q16\magick.exe"
    )

    foreach ($path in $candidates) {
        if (Test-Path $path) {
            return $path
        }
    }

    if (Test-Command "magick") {
        return (Get-Command "magick").Source
    }

    return $null
}

function Get-InnoSetupPath {
    # Support both user-scope and machine-scope Inno installs.
    $candidates = @(
        "${env:LOCALAPPDATA}\Programs\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles}\Inno Setup 6\ISCC.exe"
    )

    foreach ($path in $candidates) {
        if (Test-Path $path) {
            return $path
        }
    }

    if (Test-Command "ISCC.exe") {
        return (Get-Command "ISCC.exe").Source
    }

    return $null
}

function Assert-PrebuiltReleaseArtifacts {
    # Separation of concerns: this script packages an existing release build.
    # It does not invoke build-msvc-release.bat.
    if (-not (Test-Path $BuildDir)) {
        throw "Release build directory not found: $BuildDir. Run build-msvc-release.bat first."
    }

    $mainExe = Join-Path $BuildBinDir "slider.exe"
    if (-not (Test-Path $mainExe)) {
        throw "Release executable not found: $mainExe. Run build-msvc-release.bat first."
    }

    # locale checks removed as slider does not currently use locales
}

function Invoke-IconGeneration {
    # Keep installer icon in sync with resources/icon.png before packaging.
    $sourcePng = Join-Path $ProjectRoot "resources\icon.png"
    $outputIco = Join-Path $ProjectRoot "resources\icon.ico"

    if (-not (Test-Path $sourcePng)) {
        throw "Source icon not found: $sourcePng"
    }

    # If icon already exists and is up to date, no external tools are required.
    $regenerate = $false
    if (-not (Test-Path $outputIco)) {
        $regenerate = $true
    }
    else {
        $pngTime = (Get-Item $sourcePng).LastWriteTime
        $icoTime = (Get-Item $outputIco).LastWriteTime
        $regenerate = $pngTime -gt $icoTime
    }

    if (-not $regenerate) {
        Write-Host "Icon already up to date: $outputIco"
        return
    }

    $magick = Get-ImageMagickPath
    if (-not $magick) {
        throw "ImageMagick (magick.exe) is required to regenerate resources/icon.ico"
    }

    Write-Host "Using ImageMagick at: $magick"
    & $magick $sourcePng -background none -define icon:auto-resize=256,128,96,64,48,32,16 $outputIco
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to generate icon at $outputIco"
    }
}

function Invoke-CPackGenerator {
    param(
        [Parameter(Mandatory=$true)]
        [string]$Generator
    )

    if (-not (Test-Command "cpack")) {
        throw "cpack not found in PATH"
    }

    if ($Generator -ieq "WIX") {
        # CPack WIX depends on WiX binaries (candle/light or wix) being available.
        $hasWixV3 = (Test-Command "candle.exe") -and (Test-Command "light.exe")
        $hasWixV4 = Test-Command "wix.exe"
        if (-not $hasWixV3 -and -not $hasWixV4) {
            throw "WiX tools not found in PATH. Install WiX Toolset before generating MSI."
        }
    }

    Push-Location $BuildDir
    try {
        # Run one explicit generator at a time for deterministic outputs.
        & cpack -G $Generator
        if ($LASTEXITCODE -ne 0) {
            throw "CPack failed for generator '$Generator'"
        }
    }
    finally {
        Pop-Location
    }
}

function Invoke-InnoSetup {
    $issFile = Join-Path $ProjectRoot "installer\slider.iss"
    if (-not (Test-Path $issFile)) {
        throw "Inno Setup script not found: $issFile"
    }

    $iscc = Get-InnoSetupPath
    if (-not $iscc) {
        throw "Inno Setup not found. Install it and ensure ISCC.exe is available."
    }

    Write-Host "Using Inno Setup at: $iscc"
    & $iscc $issFile
    if ($LASTEXITCODE -ne 0) {
        throw "Inno Setup compilation failed"
    }
}

function Move-PackagesToWindowsDist {
    # Normalize all Windows artifacts under dist/windows.
    New-Item -ItemType Directory -Path $WindowsDistDir -Force | Out-Null

    @("*.zip", "*.msi", "*.exe") | ForEach-Object {
        Get-ChildItem -Path $DistDir -Filter $_ -File -ErrorAction SilentlyContinue |
            Move-Item -Destination $WindowsDistDir -Force
    }
}

try {
    Write-Step "Validating prebuilt release artifacts"
    Assert-PrebuiltReleaseArtifacts

    Write-Step "Refreshing icon assets"
    Invoke-IconGeneration

    if (-not $SkipZip) {
        Write-Step "Creating ZIP package (CPack)"
        Invoke-CPackGenerator -Generator "ZIP"
    }

    if (-not $SkipMsi) {
        Write-Step "Creating MSI package (CPack/WiX)"
        Invoke-CPackGenerator -Generator "WIX"
    }

    if (-not $SkipInstaller) {
        Write-Step "Building Inno Setup installer"
        Invoke-InnoSetup
    }

    Write-Step "Collecting package outputs"
    Move-PackagesToWindowsDist

    Write-Step "Done"
    Write-Host "Packages are in: dist\\windows\\"

    if (Test-Path $WindowsDistDir) {
        Get-ChildItem $WindowsDistDir | ForEach-Object { Write-Host "  $($_.Name)" }
    }
}
catch {
    Write-Error $_.Exception.Message
    exit 1
}
