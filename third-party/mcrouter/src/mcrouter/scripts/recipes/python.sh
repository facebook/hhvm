#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

grab http://python.org/ftp/python/2.7.6/Python-2.7.6.tar.xz
tar xf Python-2.7.6.tar.xz
cd Python-2.7.6
./configure --prefix="$INSTALL_DIR" --enable-unicode=ucs4 --enable-shared \
    LDFLAGS="-Wl,-rpath /usr/local/lib" && make $MAKE_ARGS && make altinstall $MAKE_ARGS
