# scripts/helpers/common.ps1

function Get-VSInstallationPath {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vsWhere)) {
        return $null
    }
    
    # Prefer the latest stable version. Explicitly avoid 15.x (VS 2017) if possible.
    $installations = & $vsWhere -all -prerelease -products * -format json | ConvertFrom-Json
    
    # Sort by version descending and pick the first one that is >= 16.0 (VS 2019+)
    $latest = $installations | Where-Object { [version]$_.installationVersion -ge [version]"16.0" } | Sort-Object installationVersion -Descending
    
    if ($latest) {
        return $latest[0].installationPath
    }
    
    return $null
}

function Get-VcvarsallPath {
    $vsPath = Get-VSInstallationPath
    if ($vsPath) {
        $path = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"
        if (Test-Path $path) {
            return $path
        }
    }
    return $null
}

function Find-VcpkgRoot {
    if ($env:VCPKG_INSTALLATION_ROOT -and (Test-Path $env:VCPKG_INSTALLATION_ROOT)) {
        return $env:VCPKG_INSTALLATION_ROOT
    }
    
    $commonPaths = @(
        "C:\Users\me\vcpkg",
        "$env:USERPROFILE\vcpkg",
        "C:\vcpkg",
        "C:\src\vcpkg"
    )
    
    foreach ($path in $commonPaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}

function Ensure-ToolsInPath {
    # Prefer VS-bundled CMake/Ninja
    $vsPath = Get-VSInstallationPath
    if ($vsPath) {
        $vsCmake = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
        if (Test-Path $vsCmake) {
            $env:PATH = "$vsCmake;$env:PATH"
        }
        $vsNinja = Join-Path $vsPath "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        if (Test-Path $vsNinja) {
            $env:PATH = "$vsNinja;$env:PATH"
        }
    }

    # Fallback to MSYS2
    if (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
        if (Test-Path "C:\msys64\usr\bin\ninja.exe") {
            $env:PATH += ";C:\msys64\usr\bin"
        }
    }
}

