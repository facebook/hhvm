# syntax=docker/dockerfile:1.1-experimental

FROM            ubuntu:focal

ENV             MCROUTER_DIR            /usr/local/mcrouter
ENV             MCROUTER_REPO           https://github.com/facebook/mcrouter.git
ENV             DEBIAN_FRONTEND         noninteractive

ENV             PKG_DIR                 $MCROUTER_DIR/pkgs
ENV             INSTALL_DIR             $MCROUTER_DIR/install

RUN             --mount=type=bind,target=/tmp/scripts,source=scripts /tmp/scripts/deps.sh
RUN             --mount=type=bind,target=/tmp/scripts,source=scripts /tmp/scripts/build.sh

ENV             DEBIAN_FRONTEND newt
