#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

## Set version
cd "$SCRIPT_DIR"/../
echo -n $(git tag|tail -1) > "$SCRIPT_DIR"/../VERSION

## Build
source /opt/rh/gcc-toolset-9/enable
sed -i '/m4_append.*mcrouter_version.*/d' "$SCRIPT_DIR"/../configure.ac
"$SCRIPT_DIR"/get_and_build_everything.sh centos-7.2 "$MCROUTER_DIR"

## Cleanup
rm -rf "$MCROUTER_DIR/pkgs"
rm -rf "$INSTALL_DIR/lib/*.a"
rm -rf "$INSTALL_DIR/include"

dnf remove -y   git \
                cmake \
                gcc-toolset* \
                autoconf \
                which \
                make \
                automake \
                wget \
                file \
                binutils-devel \
                glibc-devel \
                bzip2-devel \
                xz-devel \
                openssl-devel \
                python3-devel \
                gtest-devel \
                python3-pkgconfig

rm -rf "$MCROUTER_DIR/repo"
