#!/usr/bin/env sh

set -eu

clang -Os -std=c17 -Wall -Wextra -fno-strict-aliasing -fwrapv -Werror -o cx.exe main.c base/base.c

