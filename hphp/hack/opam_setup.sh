#!/bin/sh
set -euf
OCAML_PREFIX=$(dirname "$1")
SRC=$2
export PATH="$OCAML_PREFIX:$PATH"
export OPAMROOT="$SRC/_build/.opam"
mkdir -p "$OPAMROOT"
export OPAMYES="1"
MINI_TARBALL="$SRC/facebook/opam-mini-repo/opam-mini-repository.tar.gz"
MINI_REPO="$SRC/_build/opam-mini-repository"

if [ -f "$MINI_TARBALL" ]
then
	rm -rf "$MINI_REPO" ||:
	tar xzf "$MINI_TARBALL" -C "$SRC/_build"
	opam init offline_clone "$MINI_REPO" --no-setup
else
	opam init --no-setup
fi

eval "$(opam config env)"
opam install dune.1.2.1 core_kernel.v0.11.1 ppx_deriving.4.2.1 visitors.20180513 pcre.7.3.4
