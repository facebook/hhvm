#!/bin/bash
set -euf

OCAML_PREFIX=$(dirname "$1")
ROOT="$2"
export PATH="$OCAML_PREFIX:$PATH"
export OPAMROOT="$ROOT/_build/.opam"
mkdir -p "$OPAMROOT"
export OPAMYES="1"

# Shamelessly copied from
# https://github.com/facebook/infer/blob/master/scripts/opam_utils.sh
# Many thanks to the infer team :D 

opam_require_version_2 () {
    local status=0
    local version=0
    { version=$(opam --version 2>/dev/null); status=$?; }
    if [ "$status" != 0 ]; then
        printf '*** ERROR: `opam --version` failed, please install opam version 2\n' >&2
        env >&2
        exit 1
    fi
    case $version in
        2*) ;;
        *)
            printf '*** ERROR: opam version "%s" is not supported, please install opam version 2\n' "$version" >&2
            printf '*** NOTE: opam is "%s"\n' "$(command -v opam)" >&2
            env >&2
            exit 1
    esac
}

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
    fi
}

opam_require_version_2

# End of shame

HACK_OPAM_SWITCH="ocaml-base-compiler.4.05.0"
HACK_OPAM_DEFAULT_NAME="hack-switch"
HACK_OPAM_NAME=${HACK_OPAM_NAME:-$HACK_OPAM_DEFAULT_NAME}

MINI_TARBALL="$ROOT/facebook/opam2-mini-repository.tar.gz"
MINI_REPO="$ROOT/_build/opam2-mini-repository"

if [ -f "$MINI_TARBALL" ]
then
    rm -rf "$MINI_REPO" ||:
    tar xzf "$MINI_TARBALL" -C "$ROOT/_build"
    opam init --reinit offline_clone "$MINI_REPO" --no-setup --bare
else
    opam init --reinit --no-setup
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
