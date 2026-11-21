@echo off
rem Builds build\BCGrid.aex (x64). Needs Visual Studio 2022 (C++ workload or Build Tools)
rem and the Adobe After Effects SDK unpacked anywhere under .\sdk  (see sdk\README.md).
setlocal EnableExtensions
chcp 1252 >nul
set "ROOT=%~dp0"
set "OUT=%ROOT%build"

rem --- locate the SDK -------------------------------------------------------
set "HDR="
for /f "delims=" %%F in ('dir /s /b "%ROOT%sdk\AE_Effect.h" 2^>nul') do if not defined HDR set "HDR=%%~dpF"
if not defined HDR goto :no_sdk
set "HDR=%HDR:~0,-1%"
for %%I in ("%HDR%\..") do set "EX=%%~fI"
set "PIPLTOOL=%EX%\Resources\PiPLtool.exe"
if not exist "%PIPLTOOL%" goto :no_pipltool
echo [BCGrid] SDK headers: %HDR%

rem --- make sure the x64 MSVC tools are on PATH ------------------------------
set "PF86=%ProgramFiles(x86)%"
set "VSWHERE=%PF86%\Microsoft Visual Studio\Installer\vswhere.exe"
where cl >nul 2>nul && goto :have_cl
if not exist "%VSWHERE%" goto :no_vs
set "VSDIR="
for /f "usebackq delims=" %%V in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%V"
if not defined VSDIR goto :no_vs
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul
where cl >nul 2>nul || goto :no_vs
:have_cl

if not exist "%OUT%" mkdir "%OUT%"

rem --- PiPL resource (same three steps as Adobe's sample projects) -----------
cl /nologo /I "%HDR%" /EP /Tc "%ROOT%src\BCGridPiPL.r" > "%OUT%\BCGridPiPL.rr" || goto :fail
"%PIPLTOOL%" "%OUT%\BCGridPiPL.rr" "%OUT%\BCGridPiPL.rrc" || goto :fail
cl /nologo /D "MSWindows" /EP /Tc "%OUT%\BCGridPiPL.rrc" > "%OUT%\BCGridPiPL.rc" || goto :fail
rc /nologo /c 1252 /fo "%OUT%\BCGridPiPL.res" "%OUT%\BCGridPiPL.rc" || goto :fail
rc /nologo /c 1252 /fo "%OUT%\BCGridVersion.res" "%ROOT%src\BCGridVersion.rc" || goto :fail

rem --- compile and link (static CRT: no Visual C++ redistributable needed) ---
cl /nologo /c /O2 /MT /EHsc /std:c++17 /W3 /wd4068 /D "MSWindows" /D "WIN32" /D "_WINDOWS" /D "_CRT_SECURE_NO_WARNINGS" /I "%HDR%" /I "%HDR%\SP" /I "%EX%\Util" /I "%ROOT%src" /Fo"%OUT%\BCGrid.obj" "%ROOT%src\BCGrid.cpp" || goto :fail
link /nologo /DLL /MACHINE:X64 /OUT:"%OUT%\BCGrid.aex" "%OUT%\BCGrid.obj" "%OUT%\BCGridPiPL.res" "%OUT%\BCGridVersion.res" kernel32.lib user32.lib || goto :fail

echo [BCGrid] Built %OUT%\BCGrid.aex
exit /b 0

:no_sdk
echo [BCGrid] After Effects SDK not found. Unzip it into "%ROOT%sdk\" first (see sdk\README.md).
exit /b 1

:no_pipltool
echo [BCGrid] PiPLtool.exe not found at "%PIPLTOOL%".
exit /b 1

:no_vs
echo [BCGrid] Visual Studio C++ x64 tools not found. Install "Build Tools for Visual Studio 2022" with "Desktop development with C++".
exit /b 1

:fail
echo [BCGrid] Build failed.
exit /b 1
