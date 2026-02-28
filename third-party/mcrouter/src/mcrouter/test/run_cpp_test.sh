#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

ROOT_PATH="$(pwd)"
SCRIPT_PATH="$(dirname "$0")"
cd "$SCRIPT_PATH"/../.. && "$ROOT_PATH/$1"
