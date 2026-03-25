@echo off
REM Clean generated build artifacts via clean.ps1.
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0clean.ps1" %*
set "rc=%errorlevel%"
endlocal & exit /b %rc%
