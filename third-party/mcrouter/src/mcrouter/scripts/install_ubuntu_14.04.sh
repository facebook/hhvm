#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

[ -n "$1" ] || ( echo "Install dir missing"; exit 1 )

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo add-apt-repository -y ppa:george-edison55/cmake-3.x
sudo add-apt-repository -y ppa:chris-lea/libsodium
sudo add-apt-repository -y ppa:sickpig/boost

sudo apt-get update

sudo apt-get install -y \
    autoconf \
    binutils-dev \
    bison \
    cmake \
    flex \
    g++ \
    g++-5 \
    gcc \
    git \
    libboost1.58-dev \
    libboost-thread1.58-dev \
    libboost-filesystem1.58-dev \
    libboost-context1.58-dev \
    libboost-regex1.58-dev \
    libboost-program-options1.58-dev \
    libbz2-dev \
    libdouble-conversion-dev \
    libevent-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libjemalloc-dev \
    liblz4-dev \
    liblzma-dev \
    liblzma5 \
    libsnappy-dev \
    libsodium-dev \
    libssl-dev \
    libtool \
    make \
    pkg-config \
    python-dev \
    ragel \
    software-properties-common

# sudo apt-get upgrade -yq cmake

cd "$(dirname "$0")" || ( echo "cd fail"; exit 1 )

export CC="gcc-5"
export CXX="g++-5"

./get_and_build_everything.sh ubuntu-14.04 "$@"
