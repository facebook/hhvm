#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

PYTHON_PATH="$1"
TEST_NAME="$(basename "$2")"
TEST_MODULE="${TEST_NAME%.*}"
SCRIPT_PATH="$(dirname "$0")"
cd "$SCRIPT_PATH"/../.. && $PYTHON_PATH -B -m unittest -q mcrouter.test."$TEST_MODULE"
