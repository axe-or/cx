#!/usr/bin/env sh

cc=clang
cflags='-std=c17 -Os -fno-strict-aliasing -fwrapv'
wflags='-Wall -Wextra -Werror -Wno-error=unused-variable'

set -xeu

$cc $cflags $wflags -o cx.exe main.c base/base.c cx.c

