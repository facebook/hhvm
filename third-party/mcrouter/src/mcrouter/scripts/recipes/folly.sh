#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d folly ]]; then
  git clone https://github.com/facebook/folly
  cd "$PKG_DIR/folly" || die "cd fail"
  if [[ -f "$REPO_BASE_DIR/mcrouter/FOLLY_COMMIT" ]]; then
    FOLLY_COMMIT="$(head -n 1 "$REPO_BASE_DIR/mcrouter/FOLLY_COMMIT")"
    echo "FOLLY_COMMIT file found: using folly commit $FOLLY_COMMIT"
    git checkout "$FOLLY_COMMIT"
  else
    echo "No FOLLY_COMMIT file, using folly HEAD=$(git rev-parse HEAD)"
  fi
fi

cd "$PKG_DIR/folly/folly/" || die "cd fail"

CXXFLAGS="$CXXFLAGS -fPIC" \
    LD_LIBRARY_PATH="$INSTALL_DIR/lib:$LD_LIBRARY_PATH" \
    LD_RUN_PATH="$INSTALL_DIR/lib:$LD_RUN_PATH" \
    cmake .. \
    -G Ninja \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_INCLUDE_PATH="$INSTALL_DIR/lib" \
    -DCMAKE_LIBRARY_PATH="$INSTALL_DIR/lib"
cmake --build . && cmake --install .
