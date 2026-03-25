@echo off
REM Run test presets for available Windows builds.
powershell -ExecutionPolicy Bypass -File scripts\build\test-windows.ps1
