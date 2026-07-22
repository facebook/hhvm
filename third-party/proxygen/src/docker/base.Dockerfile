# Reusable proxygen "base" builder image.
#
# Builds proxygen and its full dependency tree (folly, fizz, wangle, mvfst, ...)
# from the checked-out source with getdeps.py and installs everything under a
# single CMake prefix at /opt/proxygen. Downstream projects compile against it
# with no extra flags -- CMAKE_PREFIX_PATH and LD_LIBRARY_PATH are preset:
#
#   FROM ghcr.io/facebook/proxygen/base:latest AS build
#   COPY CMakeLists.txt /app/CMakeLists.txt
#   COPY src /app/src
#   RUN cmake -S /app -B /app/build -DCMAKE_BUILD_TYPE=Release && \
#       cmake --build /app/build -j "$(nproc)"
#   # find_package(proxygen CONFIG REQUIRED) just works.
#
# Published by .github/workflows/publish_base_image.yml.
# Build locally from the repo root:
#   docker build -t proxygen-base -f docker/base.Dockerfile .

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
# getdeps installs Python build helpers system-wide inside the container.
ENV PIP_BREAK_SYSTEM_PACKAGES=1

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git g++ make libssl-dev m4 \
      python3 python3-pip pkg-config sudo ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build proxygen and all dependencies from the checked-out source.
COPY . /proxygen
WORKDIR /proxygen
# getdeps' install-system-deps runs `apt-get install` without an `apt-get
# update`, so refresh the package index (cleared in the layer above) first.
RUN apt-get update \
    && ./build/fbcode_builder/getdeps.py install-system-deps --recursive proxygen \
    && rm -rf /var/lib/apt/lists/*
RUN ./build/fbcode_builder/getdeps.py build --allow-system-packages proxygen

# Merge proxygen + every dependency install tree into one CMake prefix, so
# downstream `find_package(proxygen CONFIG)` resolves with zero extra flags.
RUN set -eux; \
    SCRATCH="$(./build/fbcode_builder/getdeps.py show-scratch-dir)"; \
    mkdir -p /opt/proxygen; \
    for d in "$SCRATCH"/installed/*/; do cp -a "$d". /opt/proxygen/; done

# Consumers inherit these, so no -DCMAKE_PREFIX_PATH / LD_LIBRARY_PATH needed.
ENV CMAKE_PREFIX_PATH=/opt/proxygen
ENV LD_LIBRARY_PATH=/opt/proxygen/lib
