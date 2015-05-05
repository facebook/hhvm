#!/bin/sh

DIR="$( cd "$( dirname "$0" )" && pwd )"

ROOT="$DIR/../../.."

./clang++ \
    -cc1 \
    -std=c++0x \
    -load "$ROOT/_build/dbg/hphp/tools/clang-gc-tool/libclang-gc-tool.so" \
    -plugin add-scan-methods \
    "$@"
