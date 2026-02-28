# syntax=docker/dockerfile:1.1-experimental

FROM    almalinux:latest@sha256:a21996e1faa92e26a48c7fe3ae5652efc4b0eacb4ead710f0cca9167251dd27e as builder

## Set desired version before building
ARG MCROUTER_VERSION

ENV     MCROUTER_DIR            /usr/local/mcrouter
ENV     INSTALL_DIR             $MCROUTER_DIR/install
ENV     SCRIPT_DIR              $MCROUTER_DIR/repo/mcrouter/mcrouter/scripts
ENV     MCROUTER_REPO           https://github.com/facebook/mcrouter.git

ENV     BOOST_VERSION="1.71.0"
ENV     FMT_VERSION="8.1.1"
ENV     SNAPPY_VERSION="1.1.9"
ENV     GFLAGS_VERSION="v2.2.2"
ENV     BISON_VERSION="v3.8.2"

RUN     --mount=type=bind,target=/tmp/scripts,source=scripts /tmp/scripts/build_deps.sh $MCROUTER_DIR
RUN     --mount=type=bind,target=/tmp/scripts,source=scripts /tmp/scripts/build.sh

FROM    almalinux:latest@sha256:a21996e1faa92e26a48c7fe3ae5652efc4b0eacb4ead710f0cca9167251dd27e

ENV     MCROUTER_DIR            /usr/local/mcrouter
ENV     INSTALL_DIR             $MCROUTER_DIR/install

COPY    --from=builder /usr/local/mcrouter /usr/local/mcrouter

RUN     --mount=type=bind,target=/tmp/scripts,source=scripts /tmp/scripts/runtime_deps.sh $MCROUTER_DIR
ENV     LD_LIBRARY_PATH         "$INSTALL_DIR/lib64:$INSTALL_DIR/lib:$LD_LIBRARY_PATH"
