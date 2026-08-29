@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0YWD-PLUG.ps1" -Action run %*
exit /b %ERRORLEVEL%
