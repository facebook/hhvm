#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

ORDER="$1"
PKG_DIR="${2%/}"/pkgs
INSTALL_DIR="${2%/}"/install
INSTALL_AUX_DIR="${2%/}"/install/aux
shift 2

mkdir -p "$PKG_DIR" "$INSTALL_DIR" "$INSTALL_AUX_DIR"

cd "$(dirname "$0")" || ( echo "cd fail"; exit 1 )

REPO_BASE_DIR="$(cd ../../ && pwd)" || die "Couldn't determine repo top dir"
export REPO_BASE_DIR

export LDFLAGS="-ljemalloc $LDFLAGS"

for script in $(ls "order_$ORDER/" | egrep '^[0-9]+_.*[^~]$' | sort -n); do
  "./order_$ORDER/$script" "$PKG_DIR" "$INSTALL_DIR" "$INSTALL_AUX_DIR" "$@"
done

printf "%s\n" "Mcrouter installed in $INSTALL_DIR/bin/mcrouter"
