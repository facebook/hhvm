#!/bin/bash -e
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cd "$(dirname "$0")"

# buildkit truncates log output.
# https://stackoverflow.com/questions/65819424/is-there-a-way-to-increase-the-log-size-in-docker-when-building-a-container
export DOCKER_BUILDKIT=0

DB="docker build --progress plain"

UBUNTU_VERSIONS="18 20 22"

# There may be an opportunity to run some of these in parallel.
for uv in $UBUNTU_VERSIONS; do
    $DB --build-arg "UBUNTU_VERSION=$uv.04" -t "watchman-ubuntu-$uv-env" ubuntu-env

    $DB --build-arg "BASE_IMAGE=watchman-ubuntu-$uv-env" -t "watchman-ubuntu-$uv-build" watchman-build

    $DB --build-arg "BASE_IMAGE=watchman-ubuntu-$uv-build" -t "watchman-ubuntu-$uv-deb" watchman-deb

    mkdir -p "_debs/ubuntu-$uv"

    docker run --rm --mount "type=bind,source=$(pwd)/_debs,target=/_out" "watchman-ubuntu-$uv-deb" sh -c "cp /_debs/* /_out/ubuntu-$uv"
done
