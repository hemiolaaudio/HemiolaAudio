#!/bin/bash
# Builds and runs the logic tests with a plain C++ compiler. Works on the Mac (Command Line Tools) and on Linux.
set -e
cd "$(dirname "$0")"
for t in test_core test_whoa test_whoa_fx; do
	OUT="${TMPDIR:-/tmp}/hemiola_$t"
	${CXX:-c++} -std=c++11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined $t.cpp -o "$OUT"
	"$OUT"
done
