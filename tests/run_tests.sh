#!/bin/bash
# Builds and runs the logic tests with a plain C++ compiler. Works on the Mac (Command Line Tools) and on Linux.
set -e
cd "$(dirname "$0")"
OUT="${TMPDIR:-/tmp}/hemiola_tests"
${CXX:-c++} -std=c++11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined test_core.cpp -o "$OUT"
"$OUT"
