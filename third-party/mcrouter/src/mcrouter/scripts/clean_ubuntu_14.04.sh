#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

sudo apt-get install -y libdouble-conversion1 libgflags2 \
    libboost-program-options1.54.0 libboost-filesystem1.54.0 \
    libboost-system1.54.0 libboost-regex1.54.0 libboost-thread1.54.0 \
    libboost-context1.54.0 libgoogle-glog0 libevent-2.0-5 libjemalloc1

sudo apt-get purge -y gcc g++ ragel autoconf \
    git libtool python-dev libssl-dev libevent-dev \
    binutils-dev make libdouble-conversion-dev libgflags-dev \
    libgoogle-glog-dev libjemalloc-dev

sudo apt-get purge -y 'libboost.*-dev'
sudo apt-get autoremove --purge -y
sudo apt-get autoclean -y
sudo apt-get clean -y

if [[ "x$1" != "x" ]]; then
    PKG_DIR=$1/pkgs
    INSTALL_DIR=$1/install
    strip "$INSTALL_DIR"/bin/mcrouter
    rm -rf "$PKG_DIR"
    rm -rf "$INSTALL_DIR"/lib/*.a
    rm -rf "$INSTALL_DIR"/include
fi
