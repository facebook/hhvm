#!/bin/sh

# Copyright (c) 2019, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

HACK_DIR="$(realpath "$(dirname "$0")")"

# cleanup OSS locations
if [ -d "${HACK_DIR}/_build" ]; then
  rm -rf "${HACK_DIR}/_build/opam"
fi

# cleanup FB locations
if [ -d "${HACK_DIR}/facebook" ]; then
  rm -rf "${HACK_DIR}/facebook/redirect/opam"
  rm -rf "${HACK_DIR}/facebook/opam2-mini-repository"
fi
