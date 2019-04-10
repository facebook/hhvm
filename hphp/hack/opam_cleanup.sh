#!/bin/sh

# Copyright (c) 2019, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

# cleanup OSS locations
if [ -d _build ]; then
  rm -rf _build/opam
fi

# cleanup FB locations
if [ -d facebook ]; then
  rm -rf facebook/opam
  rm -rf facebook/opam2-mini-repository
fi
