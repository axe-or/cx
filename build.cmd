@echo off

REM clang Build version (recommended)
clang -Os -std=c17 -fsanitize=address -Wall -Wextra -fno-strict-aliasing -fwrapv -Werror -Wno-error=unused-variable -Wno-error=unused-const-variable -o cx.exe main.c base\base.c
if %errorlevel% neq 0 exit /b %errorlevel%

REM cl Build version
REM cl /nologo /std:c17 /experimental:c11atomics /Os /EHsc /GR /W4 /Fekielo.exe main.c base\base.c 
if %errorlevel% neq 0 exit /b %errorlevel%

