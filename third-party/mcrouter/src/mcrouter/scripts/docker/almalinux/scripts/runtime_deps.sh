#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -ex

## Install runtime dependencies
dnf update -y
dnf install -y \
            double-conversion-devel \
            jemalloc-devel \
            libunwind \
            pkgconf-pkg-config \
            lz4-devel \
            xz-libs \
            libevent-devel \
            snappy-devel \
            libsodium-devel \
            libicu

## Create mcrouter default dirs
mkdir -p /var/mcrouter/stats
mkdir -p /var/mcrouter/fifos
mkdir -p /var/mcrouter/config
chgrp -R 0 /var/mcrouter
chmod -R g=u /var/mcrouter

## Create spooldir
mkdir -p /var/spool/mcrouter
chgrp -R 0 /var/spool/mcrouter
chmod -R g=u /var/spool/mcrouter

## Make runnable from any context
ln -s "$INSTALL_DIR/bin/mcrouter" /usr/bin/mcrouter
echo "export LD_LIBRARY_PATH=\"$INSTALL_DIR/lib64:$INSTALL_DIR/lib:$LD_LIBRARY_PATH\"" >> /etc/profile.d/mcrouter_libs.sh
chmod +x /etc/profile.d/mcrouter_libs.sh
echo "export LD_PRELOAD=/usr/lib64/libjemalloc.so.2" >> ~/.bashrc
