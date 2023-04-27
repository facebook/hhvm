#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d wangle ]]; then
  git clone https://github.com/facebook/wangle
  cd "$PKG_DIR/wangle" || die "cd fail"
  if [[ -f "$REPO_BASE_DIR/mcrouter/WANGLE_COMMIT" ]]; then
    WANGLE_COMMIT="$(head -n 1 "$REPO_BASE_DIR/mcrouter/WANGLE_COMMIT")"
    echo "WANGLE_COMMIT file found: using wangle commit $WANGLE_COMMIT"
    git checkout "$WANGLE_COMMIT"
  else
    echo "No WANGLE_COMMIT file, using wangle HEAD=$(git rev-parse HEAD)"
  fi
fi

cd "$PKG_DIR/wangle/wangle/" || die "cd fail"

cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DBUILD_TESTS=OFF
make $MAKE_ARGS && make install $MAKE_ARGS
