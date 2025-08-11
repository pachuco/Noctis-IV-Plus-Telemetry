@echo off

call getcomp rosbe

set sdl3base=C:\p_files\prog\_proj\CodeCocks\3rdparty_libs\SDL3
set sdl3netbase=C:\p_files\prog\_proj\CodeCocks\3rdparty_libs\SDL3_net

set opts=-std=c11 -mconsole -O3 -s -Wall -Wextra
set linkinc=-L%sdl3base%\lib -I%sdl3base%\inc -I%sdl3netbase%\inc
set linkinc=%linkinc% -lSDL3 -lws2_32 -liphlpapi

set compiles=main.c graphics.c %sdl3netbase%\src\SDL_net.c
set errlog=.\testclient.log
set out=.\testclient.exe

call :compile
call :checkerr

xcopy %sdl3base%\bin\SDL3.dll .\ /c /Y

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