#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

source common.sh

if [[ ! -d fizz ]]; then
  git clone https://github.com/facebookincubator/fizz
  cd "$PKG_DIR/fizz" || die "cd fail"
  if [[ -f "$REPO_BASE_DIR/mcrouter/FIZZ_COMMIT" ]]; then
    FIZZ_COMMIT="$(head -n 1 "$REPO_BASE_DIR/mcrouter/FIZZ_COMMIT")"
    echo "FIZZ_COMMIT file found: using fizz commit $FIZZ_COMMIT"
    git checkout "$FIZZ_COMMIT"
  else
    echo "No FIZZ_COMMIT file, using fizz HEAD=$(git rev-parse HEAD)"
  fi
fi

cd "$PKG_DIR/fizz/fizz/" || die "cd fail"

cmake . -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DBUILD_TESTS=OFF
make $MAKE_ARGS && make install $MAKE_ARGS
