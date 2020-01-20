#!/bin/bash

# Copyright (c) 2017, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

set -euf

unset DUNE_BUILD_DIR
OCAML_PREFIX=$(dirname "$1")
SOURCE_ROOT="$2"
BUILD_ROOT="${3:-"${SOURCE_ROOT}/_build"}"
export PATH="$OCAML_PREFIX:$PATH"
# detect if we are building inside FB by checking a specific dune file
if [ -e "$SOURCE_ROOT/src/facebook/dune" ]; then
  OPAMROOT="$SOURCE_ROOT/facebook/opam"
else
  OPAMROOT="${BUILD_ROOT}/opam"
fi
export OPAMROOT="$OPAMROOT"
mkdir -p "$OPAMROOT"
export OPAMYES="1"

# shellcheck disable=SC1090
source "$SOURCE_ROOT/opam_helpers.sh"

# Shamelessly copied from
# https://github.com/facebook/infer/blob/master/scripts/opam_utils.sh
# Many thanks to the infer team :D

# assumes opam is available and initialized
opam_switch_create_if_needed () {
    local name=$1
    local switch=$2
    local switch_exists=no
    for installed_switch in $(opam switch list --short); do
        if [ "$installed_switch" == "$name" ]; then
            switch_exists=yes
            break
        fi
    done
    if [ "$switch_exists" = "no" ]; then
        opam switch create "$name" "$switch"
        eval "$(opam env)"
    fi
}

opam_require_version_2

# End of shame

HACK_OPAM_SWITCH="ocaml-base-compiler.4.07.1"
HACK_OPAM_DEFAULT_NAME="hack-switch"
HACK_OPAM_NAME=${HACK_OPAM_NAME:-$HACK_OPAM_DEFAULT_NAME}

MINI_TARBALL="$SOURCE_ROOT/facebook/opam2-mini-repository.tar.gz"
MINI_REPO="$SOURCE_ROOT/facebook/opam2-mini-repository"

# OSS does not provide bubblewrap yet so we disable it
if [ -f "$MINI_TARBALL" ]
then
  rm -rf "$MINI_REPO" ||:
  tar xzf "$MINI_TARBALL" -C "$SOURCE_ROOT/facebook"
  opam init --disable-sandboxing --reinit offline_clone "$MINI_REPO" --no-setup --bare
else
  opam init --disable-sandboxing --reinit --no-setup --bare
fi

opam_switch_create_if_needed "$HACK_OPAM_NAME" "$HACK_OPAM_SWITCH"
opam switch set "$HACK_OPAM_NAME"
eval "$(opam env)"

opam install \
  core_kernel.v0.11.1 \
  dtoa.0.3.1 \
  dune.1.11.0 \
  fileutils.0.5.3 \
  lwt.4.2.1 \
  lwt_log.1.1.0 \
  lwt_ppx.1.2.2 \
  merlin.3.3.2 \
  ocp-indent.1.7.0 \
  ounit.2.2.1 \
  pcre.7.3.5 \
  ppx_deriving.4.2.1 \
  ppx_gen_rec.1.1.0 \
  sedlex.1.99.4 \
  sexplib.v0.11.0 \
  sqlite3.4.4.1 \
  uchar.0.0.2 \
  visitors.20180513 \
  wtf8.1.0.1

dune_version=$(dune --version)
echo ""
echo "opam switch correctly installed at $OPAMROOT"
echo "dune version is $dune_version"
