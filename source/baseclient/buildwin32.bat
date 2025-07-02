@echo off

call getcomp rosbe

set opts=-std=c11 -mconsole -O3 -s -Wall -Wextra
set linkinc=-lws2_32 -lwinmm

set compiles=main.c
set errlog=.\testclient.log
set out=.\testclient.exe

call :compile
call :checkerr

exit /B 0





:compile
del %out%
gcc -o %out% %compiles% %opts% %linkinc% 2> %errlog%
exit /B 0

:checkerr
IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end
exit /B 0