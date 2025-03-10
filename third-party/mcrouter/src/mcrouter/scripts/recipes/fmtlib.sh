#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d "$PKG_DIR/fmt" ]]; then
  git clone --depth 1 -b 11.0.2 https://github.com/fmtlib/fmt.git
  cd "$PKG_DIR/fmt" || die "cd failed"
  mkdir "$PKG_DIR/fmt/build"
fi

cd "$PKG_DIR/fmt/build" || die "cd fmt failed"

CXXFLAGS="$CXXFLAGS -fPIC" \
  cmake .. -G Ninja -DFMT_TEST=Off -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
cmake --build . && cmake --install .
