#!/bin/bash
#
# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# setup the build dir
TOP_DIR=$(pwd)

# setup the out dir
BUILD_DIR=out
mkdir -p $BUILD_DIR
cd $BUILD_DIR || exit
BWD=$(pwd)

DEPS_DIR=$BWD/deps

FOLLY_DIR=$DEPS_DIR/folly
FOLLY_BUILD_DIR=$DEPS_DIR/folly/build/
FIZZ_BUILD_DIR=$BWD/build

NCPU=$(sysctl -n hw.ncpu || printf 1)

mkdir -p "$FIZZ_BUILD_DIR"

# OpenSSL dirs. If you have OpenSSL installed somewhere
# else, change these dirs
OPENSSL_ROOT_DIR=/usr/local/opt/openssl
OPENSSL_LIB_DIR=/usr/local/opt/openssl/lib/


if [ -z "$INSTALL_PREFIX" ]; then
  FOLLY_INSTALL_DIR=$DEPS_DIR
  FIZZ_INSTALL_DIR=$BWD
else
  FOLLY_INSTALL_DIR=$INSTALL_PREFIX
  FIZZ_INSTALL_DIR=$INSTALL_PREFIX
fi

if [ ! -d "$FOLLY_DIR" ] ; then
  # install the default dependencies from homebrew
  brew install \
    cmake \
    boost \
    double-conversion \
    fmt \
    gflags \
    glog \
    libevent \
    lz4 \
    snappy \
    xz \
    openssl \
    libsodium

  brew link \
    boost \
    double-conversion \
    gflags \
    fmt \
    glog \
    libevent \
    lz4 \
    snappy \
    xz \
    libsodium

  # build folly
  git clone https://github.com/facebook/folly.git "$FOLLY_DIR"
  echo "Building Folly ($NCPU cores)"
  mkdir -p "$FOLLY_BUILD_DIR"
  cd "$FOLLY_BUILD_DIR" || exit
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX="$FOLLY_INSTALL_DIR" \
    -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR" \
    -DOPENSSL_LIBRARIES="$OPENSSL_LIB_DIR" ..
  make -j"${NCPU}" install
  cd "$BWD" || exit
fi

# build fizz
cd "$FIZZ_BUILD_DIR" || exit
cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX="$FIZZ_INSTALL_DIR" \
  -DCMAKE_PREFIX_PATH="$FOLLY_INSTALL_DIR" \
  -DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR" \
  -DOPENSSL_LIBRARIES="$OPENSSL_LIB_DIR" ../..

echo "Building Fizz ($NCPU cores)"
make -j"${NCPU}" install

rm -rf "${BWD:?}"/bin
cp -R "$FIZZ_BUILD_DIR"/bin/ "$BWD"/bin/

cd "$TOP_DIR" || exit
