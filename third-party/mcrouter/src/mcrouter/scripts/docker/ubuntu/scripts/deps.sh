#!/bin/bash -e
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

apt -y update
apt -y install apt-transport-https \
                ca-certificates \
                tzdata \
                git
apt clean all
