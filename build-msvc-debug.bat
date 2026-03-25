@echo off
REM Build Markdown Viewer using MSVC in Debug mode.
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\build\build-windows.ps1" -Compiler msvc -Config Debug
set "rc=%errorlevel%"
endlocal & exit /b %rc%
