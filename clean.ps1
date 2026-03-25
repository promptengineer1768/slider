[CmdletBinding()]
param(
    [switch]$KeepVcpkgInstalled
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Remove-IfExists {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath,
        [Parameter(Mandatory = $true)]
        [string]$Message
    )

    $target = Join-Path $ProjectRoot $RelativePath
    if (Test-Path -LiteralPath $target) {
        Remove-Item -LiteralPath $target -Recurse -Force -ErrorAction Stop
        Write-Host $Message
    }
}

Write-Host "Cleaning build artifacts..."

Remove-IfExists -RelativePath "build" -Message "Removed build/"
Remove-IfExists -RelativePath "dist" -Message "Removed dist/"

if (-not $KeepVcpkgInstalled) {
    Remove-IfExists -RelativePath "vcpkg_installed" -Message "Removed vcpkg_installed/"
}

Remove-IfExists -RelativePath "_CPack_Packages" -Message "Removed _CPack_Packages/"
Remove-IfExists -RelativePath "AppDir" -Message "Removed AppDir/"
Remove-IfExists -RelativePath "resources/icon.ico" -Message "Removed generated resources/icon.ico"
Remove-IfExists -RelativePath "resources/icon.icns" -Message "Removed generated resources/icon.icns"
Remove-IfExists -RelativePath "resources/icon-256.png" -Message "Removed generated resources/icon-256.png"

Write-Host "Clean complete."
