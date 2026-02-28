#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [ ! -d "$PKG_DIR/glog" ]; then
    git clone https://github.com/google/googletest.git "$PKG_DIR/gtest" \
        --branch v1.10.x --depth 1
    cd "$PKG_DIR/gtest" || die "cd fail"

    cmake . \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        -DCMAKE_INSTALL_LIBDIR="lib"
    make $MAKE_ARGS && make install $MAKE_ARGS
fi
