@echo off
REM Build Inno Setup installer for Markdown Viewer
REM This is a thin wrapper that calls the package script with -SkipBuild -SkipZip

setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\build\package-windows.ps1" -SkipBuild -SkipZip %*
set "rc=%errorlevel%"
endlocal & exit /b %rc%
