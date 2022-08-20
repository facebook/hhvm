#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d "$PKG_DIR/fbthrift" ]]; then
  git clone https://github.com/facebook/fbthrift
  cd "$PKG_DIR/fbthrift" || die "cd fail"
  if [[ -f "$REPO_BASE_DIR/mcrouter/FBTHRIFT_COMMIT" ]]; then
    FBTHRIFT_COMMIT="$(head -n 1 "$REPO_BASE_DIR/mcrouter/FBTHRIFT_COMMIT")"
    echo "FBTHRIFT_COMMIT file found: using fbthrift commit $FBTHRIFT_COMMIT"
    git checkout "$FBTHRIFT_COMMIT"
  else
    echo "No FBTHRIFT_COMMIT file, using fbthrift HEAD=$(git rev-parse HEAD)"
  fi
fi

cd "$PKG_DIR/fbthrift/build" || die "cd fbthrift failed"

CXXFLAGS="$CXXFLAGS -fPIC" \
cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
make $MAKE_ARGS && make install $MAKE_ARGS
