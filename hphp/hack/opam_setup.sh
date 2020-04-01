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
# shellcheck disable=SC1090
source "$SOURCE_ROOT/ocaml_deps_data.sh"

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


HACK_OPAM_SWITCH="ocaml-base-compiler.${HACK_OCAML_VERSION}"
HACK_OPAM_DEFAULT_NAME="hack-switch"
HACK_OPAM_NAME=${HACK_OPAM_NAME:-$HACK_OPAM_DEFAULT_NAME}
SKIP_MINI_REPO=${SKIP_MINI_REPO:-0}

if [[ "${SKIP_MINI_REPO}" -eq 1 ]]; then
  echo "SKIP_MINI_REPO is set."
  echo "This setup will fetch from the internet."
  echo "Make sure you know what you are doing."
  export http_proxy=http://fwdproxy:8080
  export https_proxy=http://fwdproxy:8080
fi

MINI_REPO_FETCH_SCRIPT="${SOURCE_ROOT}/facebook/fetch_opam2_repo_hack.sh"

# OSS does not provide bubblewrap yet so we disable it
if [[ -f "${MINI_REPO_FETCH_SCRIPT}" && "${SKIP_MINI_REPO}" -eq 0 ]]; then
  MINI_REPO_DIR="$("${MINI_REPO_FETCH_SCRIPT}")"
  MINI_REPO_TARBALL="${MINI_REPO_DIR}.tar.gz"
  rm -rf "$MINI_REPO_DIR" ||:
  tar xzf "$MINI_REPO_TARBALL" -C "$SOURCE_ROOT/facebook"
  opam init --disable-sandboxing --reinit offline_clone "$MINI_REPO_DIR" --no-setup --bare
else
  opam init --disable-sandboxing --reinit --no-setup --bare
fi

opam_switch_create_if_needed "$HACK_OPAM_NAME" "$HACK_OPAM_SWITCH"
opam switch "$HACK_OPAM_NAME"
eval "$(opam env)"

opam install "${HACK_OPAM_DEPS[@]}"

dune_version=$(dune --version)
echo ""
echo "opam switch correctly installed at $OPAMROOT"
echo "dune version is $dune_version"
