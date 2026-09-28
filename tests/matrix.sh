#!/bin/sh
# Every combination of kind, generated, configured, built and tested.
#
# The dependencies are fetched once into a shared cache: without it each case
# would pull raylib again, and the matrix would take an hour.
#
#   tests/matrix.sh [devkit binary] [work directory]

set -u

devkit=${1:-$(cd "$(dirname "$0")/.." && pwd)/build/devkit}
work=${2:-${TMPDIR:-/tmp}/devkit-matrix}
cache=$work/_deps
failures=0

rm -rf "$work" && mkdir -p "$cache"

run() {
    kinds=$1
    sandboxes=$2
    name=$(echo "k$kinds$sandboxes" | tr -d ' -')
    folder=$work/$name

    mkdir -p "$folder" && cd "$folder" || return 1

    # shellcheck disable=SC2086
    "$devkit" init -k $kinds $sandboxes >/dev/null 2>&1 || { report "$name" "init"; return 1; }

    printf 'int answer() { return 42; }\n' > sources/answer.cpp
    # the list is the repository's own, as it is for anyone adding a file
    python3 -c "
import sys
text = open('CMakeLists.txt').read()
open('CMakeLists.txt', 'w').write(text.replace('    sources/no_source.cpp', '    sources/no_source.cpp\n    sources/answer.cpp', 1))
"
    mkdir -p includes tests
    printf 'int answer();\n' > includes/Answer.hpp
    printf '#include "Answer.hpp"\n#include <gtest/gtest.h>\nTEST(A, B) { EXPECT_EQ(answer(), 42); }\n' \
        > tests/TestAnswer.cpp

    cmake -B build -S . -DFETCHCONTENT_BASE_DIR="$cache" >log 2>&1 || { report "$name" "configure"; return 1; }
    cmake --build build -j8 >>log 2>&1 || { report "$name" "build"; return 1; }
    ctest --test-dir build >>log 2>&1 || { report "$name" "ctest"; return 1; }

    printf '  %-26s ok   %s\n' "$name" "$(ls build | grep -E "^(lib)?$name" | tr '\n' ' ')"
}

report() {
    printf '  %-26s FAILED (%s)\n' "$1" "$2"
    tail -15 "$work/$1/log" | sed 's/^/      /'
    failures=$((failures + 1))
}

echo "kind combinations"
for kinds in "static" "shared" "app" "static shared" "static app" "shared app" "static shared app"; do
    run "$kinds" ""
done

echo "with a sandbox"
for kinds in "static" "app" "static shared app"; do
    run "$kinds" "--example"
done

echo
if [ "$failures" -eq 0 ]; then
    echo "every case passed"
else
    echo "$failures case(s) failed"
fi
exit "$failures"
