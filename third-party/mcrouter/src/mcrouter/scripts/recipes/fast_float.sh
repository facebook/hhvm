#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d "$PKG_DIR/fast_float" ]]; then
  git clone --depth 1 -b v8.0.2 https://github.com/fastfloat/fast_float.git
  cd "$PKG_DIR/fast_float" || die "cd failed"
  mkdir "$PKG_DIR/fast_float/build"
fi

cd "$PKG_DIR/fast_float/build" || die "cd fast_float failed"

CXXFLAGS="$CXXFLAGS -fPIC" \
  cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
cmake --build . && cmake --install .
