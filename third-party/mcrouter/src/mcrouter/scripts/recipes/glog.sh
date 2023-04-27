#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [ ! -d "$PKG_DIR/glog" ]; then
    git clone https://github.com/google/glog.git
    cd "$PKG_DIR/glog" || die "cd fail"

    autoreconf --install
    LDFLAGS="-Wl,-rpath=$INSTALL_DIR/lib,--enable-new-dtags \
             -L$INSTALL_DIR/lib $LDFLAGS" \
        CPPFLAGS="-I$INSTALL_DIR/include -DGOOGLE_GLOG_DLL_DECL='' $CPPFLAGS" \
        ./configure --prefix="$INSTALL_DIR" && \
        make $MAKE_ARGS && make install $MAKE_ARGS
fi
