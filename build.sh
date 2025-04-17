#!/usr/bin/env sh

set -eu

clang -Os -std=c17 -Wall -Wextra -fno-strict-aliasing -fwrapv -Werror -Wno-error=unused-variable -o cx.exe main.c base/base.c

