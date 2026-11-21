@echo off
rem Manual install without the setup program. Right-click > "Ejecutar como administrador".
setlocal
set "SRC=%~dp0BCGrid.aex"
if not exist "%SRC%" set "SRC=%~dp0build\BCGrid.aex"
set "DEST=%ProgramFiles%\Adobe\Common\Plug-ins\7.0\MediaCore\BillyCreative"
net session >nul 2>&1 || (echo Ejecuta este archivo como administrador. & pause & exit /b 1)
if not exist "%DEST%" mkdir "%DEST%"
copy /y "%SRC%" "%DEST%\BCGrid.aex" >nul || (echo No se pudo copiar. Cierra Premiere Pro y vuelve a intentarlo. & pause & exit /b 1)
echo BCGrid instalado en "%DEST%". Abre Premiere Pro.
pause
