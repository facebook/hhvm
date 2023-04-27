#!/bin/bash -e
# vim:ts=2:sw=2:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -x
python3 "$(dirname "$0")/build/fbcode_builder/getdeps.py" install-system-deps --recursive watchman
