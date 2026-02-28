#!/bin/bash -e
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cd "$(dirname "$0")"
cd "$(git rev-parse --show-toplevel)"

# Why /usr/local? This .deb does not hold to any rigorous packaging
# standards (e.g. Debian), and is intended to replace existing ad hoc
# Watchman installations.
#
# Until we have a reason otherwise, let's remain compatible with
# legacy installation paths.
PREFIX="/usr/local"

set -x

if [[ $BUILT = "" ]]; then
    BUILT=$(mktemp -d)
    trap 'rm -rf -- "$BUILT"' EXIT

    sudo python3 build/fbcode_builder/getdeps.py install-system-deps \
         --recursive watchman

    python3 build/fbcode_builder/getdeps.py build \
            --allow-system-packages \
            --src-dir=. \
            "--project-install-prefix=watchman:$PREFIX" \
            watchman

    python3 build/fbcode_builder/getdeps.py fixup-dyn-deps \
            --allow-system-packages \
            --src-dir=. \
            "--project-install-prefix=watchman:$PREFIX" \
            --final-install-prefix "$PREFIX" \
            watchman \
            "$BUILT"
fi

# This is an implicit assertion that the `lib` directory is empty.
# We statically link dependencies for packaging purposes.
rmdir "$BUILT/lib"

PACKAGE_VERSION=$("$BUILT/bin/watchman" --version)

PACKAGE_WORKDIR=$(mktemp -d)
trap 'rm -rf -- "$PACKAGE_WORKDIR"' EXIT

mkdir -p "$PACKAGE_WORKDIR$PREFIX"
cp -ar "$BUILT/bin" "$PACKAGE_WORKDIR$PREFIX/bin"
cp -ar watchman/build/package/watchman-deb/DEBIAN "$PACKAGE_WORKDIR"

sed -i "s/%VERSION%/$PACKAGE_VERSION/" "$PACKAGE_WORKDIR/DEBIAN/control"

mkdir -p /_debs

DEB_OUTPUT="/_debs/watchman_$PACKAGE_VERSION.deb"

# TODO: use dpkg-shlibdeps to automate generation of the runtime
# dependency list

dpkg-deb -b "$PACKAGE_WORKDIR" "$DEB_OUTPUT"
