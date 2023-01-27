@echo off
setlocal

if "%1"=="" goto :nmake
if not "%2"=="" goto :help

if /i "%1"=="/clean" goto :clean

:help
echo Usage: %0 [/clean]
goto :eof

:nmake
set nmake=nmake /nologo config=Release

%nmake% clean || goto :eof
%nmake% dist ndebug=1 reminder="Reminder: Don't forget to also make x64_or_x86 distribution."
goto :eof

:clean
for %%i in (x64 Win32) do if exist "%%i" rmdir /q /s "%%i"
