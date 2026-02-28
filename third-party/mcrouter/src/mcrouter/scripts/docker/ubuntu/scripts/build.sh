#!/bin/bash -e
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

## Build
mkdir -p $MCROUTER_DIR/repo
cd $MCROUTER_DIR/repo && git clone $MCROUTER_REPO
cd $MCROUTER_DIR/repo/mcrouter/mcrouter/scripts
sed -i 's/sudo //g' ./install_ubuntu_20.04.sh
sed -i 's/MAKE_ARGS=\"\$@\"/MAKE_ARGS=\"-j$(nproc)\"/g' ./common.sh
./install_ubuntu_20.04.sh $MCROUTER_DIR

## Cleanup
apt-get purge -y pkg-config cmake
sed -i 's/sudo //g' ./clean_ubuntu_14.04.sh
sed -i 's/libdouble-conversion1/libdouble-conversion3/g' ./clean_ubuntu_14.04.sh
sed -i 's/libgflags2/libgflags2.2/g' ./clean_ubuntu_14.04.sh
sed -i 's/1\.54\.0/1.71.0/g' ./clean_ubuntu_14.04.sh
sed -i 's/libevent-2.0-5/libevent-2.1-7/g' ./clean_ubuntu_14.04.sh
sed -i 's/libjemalloc1/libjemalloc2/g' ./clean_ubuntu_14.04.sh
sed -i 's/libgoogle-glog0/libgoogle-glog0v5/g' ./clean_ubuntu_14.04.sh
./clean_ubuntu_14.04.sh $MCROUTER_DIR && rm -rf $MCROUTER_DIR/repo

## Prepare for use
ln -s $MCROUTER_DIR/install/bin/mcrouter /usr/local/bin/mcrouter
