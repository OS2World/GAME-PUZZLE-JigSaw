@echo off
rem compile-wat.cmd -- build Jigsaw with OpenWatcom 2.0

set LOGFILE=compile-wat.log
echo. > %LOGFILE%
echo === Jigsaw OpenWatcom build === 2>&1 | tee -a %LOGFILE%

rem Auto-detect Watcom installation
if exist c:\watcom2\binp\wmake.exe goto :watcom2
if exist c:\watcom\binp\wmake.exe goto :watcom1
echo ERROR: OpenWatcom not found (tried c:\watcom2 and c:\watcom) 2>&1 | tee -a %LOGFILE%
goto :end

:watcom2
set WATCOM=c:\watcom2
goto :found

:watcom1
set WATCOM=c:\watcom
goto :found

:found
echo Using Watcom at %WATCOM% 2>&1 | tee -a %LOGFILE%
set PATH=%WATCOM%\binp;%WATCOM%\binw;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os2
set LIB=%WATCOM%\lib386;%WATCOM%\lib386\os2

if "%OS2TK%" == "" set OS2TK=c:\os2tk45

mkdir bin 2>&1 | tee -a %LOGFILE%
%WATCOM%\binp\wmake.exe -f makefile.wat clean all 2>&1 | tee -a %LOGFILE%

if exist bin\jigsaw.exe goto :ok
echo. 2>&1 | tee -a %LOGFILE%
echo *** BUILD FAILED -- see %LOGFILE% for details *** 2>&1 | tee -a %LOGFILE%
goto :end

:ok
echo. 2>&1 | tee -a %LOGFILE%
echo BUILD OK -- bin\jigsaw.exe ready 2>&1 | tee -a %LOGFILE%

:end
