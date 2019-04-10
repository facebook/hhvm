#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

set -euf

OCAML_PREFIX=$(dirname "$1")
ROOT="$2"
export PATH="$OCAML_PREFIX:$PATH"
export OPAMROOT="$ROOT/_build/.opam"
mkdir -p "$OPAMROOT"
export OPAMYES="1"

# shellcheck disable=SC1090
source "$ROOT/opam_helpers.sh"

# Shamelessly copied from
# https://github.com/facebook/infer/blob/master/scripts/opam_utils.sh
# Many thanks to the infer team :D

# assumes opam is available and initialized
opam_switch_create_if_needed () {
    local name=$1
    local switch=$2
    local source_dir="$OPAMROOT/custom-sources"
    local ocaml_dir="$source_dir/$switch"
    local switch_exists=no
    for installed_switch in $(opam switch list --short); do
        if [ "$installed_switch" == "$name" ]; then
            switch_exists=yes
            break
        fi
    done
    if [ "$switch_exists" = "no" ]; then
        # Creates an empty switch so we can fetch ocaml source code
        # and patch it for arm64 builds
        opam switch create --empty "$name"
        mkdir -p "$source_dir"
        opam source "$switch" --dir "$ocaml_dir"
        pushd "$ocaml_dir"
        patch -p1 < "$ROOT/ocaml.patch"
        popd
        opam pin add ocaml-variants.4.05.0+arm64-patch "$ocaml_dir"
        eval "$(opam env)"
    fi
}

opam_require_version_2

# End of shame

HACK_OPAM_SWITCH="ocaml-base-compiler.4.05.0"
HACK_OPAM_DEFAULT_NAME="hack-switch"
HACK_OPAM_NAME=${HACK_OPAM_NAME:-$HACK_OPAM_DEFAULT_NAME}

MINI_TARBALL="$ROOT/facebook/opam2-mini-repository.tar.gz"
MINI_REPO="$ROOT/_build/opam2-mini-repository"

# OSS does not provide bubblewrap yet so we disable it
if [ -f "$MINI_TARBALL" ]
then
  rm -rf "$MINI_REPO" ||:
  tar xzf "$MINI_TARBALL" -C "$ROOT/_build"
  opam init --disable-sandboxing --reinit offline_clone "$MINI_REPO" --no-setup --bare
else
  opam init --disable-sandboxing --reinit --no-setup --bare
fi

opam_switch_create_if_needed "$HACK_OPAM_NAME" "$HACK_OPAM_SWITCH"
opam switch set "$HACK_OPAM_NAME"
eval "$(opam env)"

opam install \
  core_kernel.v0.11.1 \
  dune.1.6.3 \
  lwt.4.1.0 \
  lwt_log.1.1.0 \
  lwt_ppx.1.2.1 \
  pcre.7.3.4 \
  ppx_deriving.4.2.1 \
  visitors.20180513
