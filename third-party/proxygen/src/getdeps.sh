#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

set -xeo pipefail

TOOLCHAIN_DIR=/opt/rh/devtoolset-8/root/usr/bin
if [[ -d "$TOOLCHAIN_DIR" ]]; then
    PATH="$TOOLCHAIN_DIR:$PATH"
fi

PROJECT_DIR=$(dirname "$0")
GETDEPS_PATHS=(
    "$PROJECT_DIR/build/fbcode_builder/getdeps.py"
    "$PROJECT_DIR/../../opensource/fbcode_builder/getdeps.py"
)

ROOT_DIR=$(pwd)
STAGE=${ROOT_DIR}/_build/
mkdir -p "$STAGE"

for getdeps in "${GETDEPS_PATHS[@]}"; do
    if [[ -x "$getdeps" ]]; then
        "$getdeps" build proxygen --current-project proxygen "$@" --install-prefix=${STAGE}
        exit 0
    fi
done

echo "Could not find getdeps.py!?" >&2
exit 1
