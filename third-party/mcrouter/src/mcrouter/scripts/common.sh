# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

function die { printf "%s: %s\n" "$0" "$@"; exit 1; }

[ -n "$1" ] || die "PKG_DIR missing"
[ -n "$2" ] || die "INSTALL_DIR missing"

PKG_DIR="$1"
INSTALL_DIR="$2"
INSTALL_AUX_DIR="$3"
shift $#
MAKE_ARGS="$@ -j$(nproc)"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p "$PKG_DIR" "$INSTALL_DIR"

if [ -n "$3" ]; then
  mkdir -p "$INSTALL_AUX_DIR"
fi

cd "$PKG_DIR" || die "cd fail"
