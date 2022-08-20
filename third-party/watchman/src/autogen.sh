#!/bin/bash -e
# vim:ts=2:sw=2:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cd "$(dirname "$0")"

set -x
PREFIX=${PREFIX:-/usr/local}
python3 build/fbcode_builder/getdeps.py build \
        --allow-system-packages \
        --src-dir=. \
        "--project-install-prefix=watchman:$PREFIX" \
        watchman
python3 build/fbcode_builder/getdeps.py fixup-dyn-deps \
        --allow-system-packages \
        --src-dir=. \
        "--project-install-prefix=watchman:$PREFIX" \
        --final-install-prefix "$PREFIX" \
        watchman built

find built -ls
