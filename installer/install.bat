@echo off
rem Manual install without the setup program. Right-click > "Run as administrator".
setlocal
set "SRC=%~dp0BCGrid.aex"
if not exist "%SRC%" set "SRC=%~dp0build\BCGrid.aex"
set "DEST=%ProgramFiles%\Adobe\Common\Plug-ins\7.0\MediaCore\BillyCreative"
net session >nul 2>&1 || (echo Please right-click this file and choose "Run as administrator". & pause & exit /b 1)
if not exist "%DEST%" mkdir "%DEST%"
copy /y "%SRC%" "%DEST%\BCGrid.aex" >nul || (echo Could not copy the plugin. Close Premiere Pro and try again. & pause & exit /b 1)
echo BC-Grid installed in "%DEST%". You can open Premiere Pro now.
pause
