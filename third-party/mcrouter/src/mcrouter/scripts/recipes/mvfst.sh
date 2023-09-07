#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [ ! -d "$PKG_DIR/mvfst" ]; then
    git clone https://github.com/facebook/mvfst.git "$PKG_DIR/mvfst"
    cd "$PKG_DIR/mvfst" || die "cd fail"

    cmake . \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DBUILD_TESTS=OFF
    make $MAKE_ARGS && make install $MAKE_ARGS
fi
