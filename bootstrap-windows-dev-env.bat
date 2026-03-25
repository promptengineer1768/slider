@echo off
REM Phase 1: Thin wrapper to bootstrap the Windows development environment.
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0scripts\bootstrap\bootstrap-windows.ps1" %*
set "rc=%errorlevel%"
endlocal & exit /b %rc%
