#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

## Install deps
dnf -y install epel-release \
                dnf-plugins-core \
                https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
dnf config-manager --set-enabled powertools
dnf update -y

dnf install -y \
    git \
    gcc-toolset-9-gcc \
    gcc-toolset-9-gcc-c++ \
    autoconf \
    binutils-devel \
    glibc-devel \
    file \
    cmake \
    wget \
    which \
    double-conversion-devel \
    flex \
    zstd \
    libzstd-devel \
    bzip2-devel \
    glog-devel \
    gtest-devel \
    jemalloc-devel \
    libtool \
    libunwind \
    libunwind-devel \
    pkgconf-pkg-config \
    python3-pkgconfig \
    lz4-devel \
    xz-devel \
    xz-libs \
    libevent-devel \
    make \
    openssl-devel \
    python3-devel \
    snappy-devel \
    libsodium-devel \
    ragel \
    automake.noarch \
    libicu \
    libicu-devel

## Download the mcrouter project
mkdir -p "$MCROUTER_DIR/repo" && \
cd "$MCROUTER_DIR/repo" && git clone --branch "$MCROUTER_VERSION" "$MCROUTER_REPO"

## Enable gcc toolset
source /opt/rh/gcc-toolset-9/enable

## Boost
dnf remove -y boost-devel
BOOST_VERSION_MOD="boost_$(echo $BOOST_VERSION | tr '.' '_')"
wget "https://boostorg.jfrog.io/artifactory/main/release/$BOOST_VERSION/source/$BOOST_VERSION_MOD.tar.gz"
tar -xzf "$BOOST_VERSION_MOD.tar.gz" && cd "$BOOST_VERSION_MOD"
./bootstrap.sh "--prefix=$INSTALL_DIR"
./b2 install "--prefix=$INSTALL_DIR"
cd - && rm -rf ./boost

## Fmt
git clone --branch "$FMT_VERSION" https://github.com/fmtlib/fmt.git fmt_lib
cd fmt_lib && cmake -DBUILD_SHARED_LIBS=TRUE -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
make -j$(nproc) && make -j$(nproc) install
cd - && rm -rf fmt_lib

## Snappy
git clone --branch "$SNAPPY_VERSION" https://github.com/google/snappy.git snappy_lib
cd snappy_lib && git submodule update --init
mkdir build && cd build && cmake ../
make -j$(nproc) && make -j$(nproc) install
cd "$SCRIPT_DIR"
rm -rf snappy_lib

## Gflags
dnf remove -y gflags gflags-devel
git clone --branch "$GFLAGS_VERSION" https://github.com/gflags/gflags.git gflags
mkdir gflags/build && cd gflags/build
export LDFLAGS="-Wl,-rpath=$INSTALL_DIR/lib,--enable-new-dtags -L$INSTALL_DIR/lib $LDFLAGS"
export CPPFLAGS="-I$INSTALL_DIR/include -DGFLAGS_DLL_DECL='' $CPPFLAGS"
cmake ../ -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DBUILD_SHARED_LIBS=ON
make -j$(nproc) && make -j$(nproc) install
cd - && rm -rf gflags

## Bison
BISON_VERION_MOD="$(echo $BISON_VERSION | egrep -o '[0-9]\.[0-9]\.[0-9]')"
wget "https://ftp.gnu.org/gnu/bison/bison-$BISON_VERION_MOD.tar.xz"
tar -xf "./bison-$BISON_VERION_MOD.tar.xz"
cd "bison-$BISON_VERION_MOD" && ./configure
make -j$(nproc) && make -j$(nproc) install
cd - && rm -rf ./"bison-$BISON_VERION_MOD"

## Install Folly deps before building folly
sed -i '/cd \"\$PKG_DIR\/folly\".*/a ./build/fbcode_builder/getdeps.py install-system-deps --recursive' "$SCRIPT_DIR/recipes/folly.sh"

## Use dnf instead of yum in fbthrift
sed -i '/dnf.*/a ./build/fbcode_builder/getdeps.py install-system-deps --recursive fbthrift' "$SCRIPT_DIR/recipes/fbthrift.sh"

## Glog
sed -i 's/git clone.*/wget https:\/\/github.com\/google\/glog\/archive\/refs\/tags\/v0.4.0.tar.gz/g' "$SCRIPT_DIR/recipes/glog.sh"
sed -i '/wget.*/a\ \ \ \ tar -zxf .\/v0.4.0.tar.gz' "$SCRIPT_DIR/recipes/glog.sh"
sed -i '/if/! s/DIR\/glog/DIR\/glog-0.4.0/g' "$SCRIPT_DIR/recipes/glog.sh"
ln -s "$SCRIPT_DIR/recipes/glog.sh" "$SCRIPT_DIR/order_centos-7.2/01_glog"

## Patching mcrouter scripts to build with new libs
sed -i '/AC_HAVE_LIBRARY.*gflags/,+1d' "${MCROUTER_DIR}/repo/mcrouter/mcrouter/configure.ac"
sed -i '/if.*mcrouter_cv_prog_cc_gflags/,+2d' "${MCROUTER_DIR}/repo/mcrouter/mcrouter/configure.ac"
sed -i 's/lib\ $LDFLAGS/lib64 -L$INSTALL_DIR\/lib $LDFLAGS/g' "$SCRIPT_DIR/recipes/mcrouter.sh"
sed -i 's/configure.*/& --with-boost-libdir=\/usr\/local\/mcrouter\/install\/lib/g' "$SCRIPT_DIR/recipes/mcrouter.sh"
sed -i 's/LD_LIBRARY_PATH="/&$INSTALL_DIR\/lib64\:/g' "$SCRIPT_DIR/recipes/mcrouter.sh"

## Pretend python 3 is installed (required for build)
ln -s /usr/bin/python3.6 /usr/bin/python
