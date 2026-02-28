#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

MAKE_FILE="$1"
TARGET="$2"
PKG_DIR="${3%/}"/pkgs
INSTALL_DIR="${3%/}"/install
INSTALL_AUX_DIR="${3%/}"/install/aux

[ -n "$MAKE_FILE" ] || ( echo "Make file missing"; exit 1 )
[ -n "$TARGET" ] || ( echo "Target missing"; exit 1 )

mkdir -p "$PKG_DIR" "$INSTALL_DIR" "$INSTALL_AUX_DIR"

cd "$(dirname "$0")" || ( echo "cd fail"; exit 1 )

REPO_BASE_DIR="$(cd ../../ && pwd)" || die "Couldn't determine repo top dir"
export REPO_BASE_DIR

export LDFLAGS="-ljemalloc $LDFLAGS"

make "$TARGET" -j3 -f "$MAKE_FILE" PKG_DIR="$PKG_DIR" INSTALL_DIR="$INSTALL_DIR" INSTALL_AUX_DIR="$INSTALL_AUX_DIR"

printf "%s\n" "make $TARGET for $MAKE_FILE done"
