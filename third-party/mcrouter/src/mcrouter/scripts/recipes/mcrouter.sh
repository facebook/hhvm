#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

cd "$SCRIPT_DIR/../.." || die "cd fail"

autoreconf --install
LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH" \
    LD_RUN_PATH="$INSTALL_DIR/lib:$LD_RUN_PATH" \
    LDFLAGS="-L$INSTALL_DIR/lib $LDFLAGS" \
    CPPFLAGS="-I$INSTALL_DIR/include $CPPFLAGS" \
    FBTHRIFT_BIN="$INSTALL_DIR/bin/" \
    ./configure --prefix="$INSTALL_DIR"
make $MAKE_ARGS && make install $MAKE_ARGS
