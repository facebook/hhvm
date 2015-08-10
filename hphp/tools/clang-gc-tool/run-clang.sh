#!/bin/sh

DIR="$( cd "$( dirname "$0" )" && pwd )"

ROOT="$DIR/../../.."

./clang++ \
    -cc1 \
    -DDEBUG \
    -load "$ROOT/_build/dbg/hphp/tools/clang-gc-tool/libclang-gc-tool.so" \
    -plugin-arg-add-scan-methods -v \
    -plugin add-scan-methods \
    -std=c++0x \
    -isystem "$ROOT/third-party2/libgcc/4.8.1/gcc-4.8.1-glibc-2.17-fb/8aac7fc/include/c++/4.8.1" \
    -isystem "$ROOT/third-party2/libgcc/4.8.1/gcc-4.8.1-glibc-2.17-fb/8aac7fc/include/c++/4.8.1/x86_64-facebook-linux" \
    -isystem "$ROOT/third-party2/libgcc/4.8.1/gcc-4.8.1-glibc-2.17-fb/8aac7fc/include/c++/4.8.1/backward" \
    -isystem "$ROOT/third-party2/glibc/2.17/gcc-4.8.1-glibc-2.17-fb/99df8fc/include" \
    -isystem "$ROOT/third-party2/clang/dev/centos6-native/af4b1a0/lib/clang/dev/include" \
    -isystem "$ROOT/third-party2/kernel-headers/3.2.18_70_fbk11_00129_gc8882d0/gcc-4.8.1-glibc-2.17-fb/da39a3e/include" \
    "$@"

