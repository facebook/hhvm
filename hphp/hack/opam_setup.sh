#!/bin/sh
set -euf
OCAML_PREFIX=$(dirname "$1")
ROOT="$2"
SRC="$ROOT/src"
export PATH="$OCAML_PREFIX:$PATH"
export OPAMROOT="$ROOT/_build/.opam"
mkdir -p "$OPAMROOT"
export OPAMYES="1"
MINI_TARBALL="$SRC/facebook/opam-mini-repo/opam-mini-repository.tar.gz"
MINI_REPO="$ROOT/_build/opam-mini-repository"

if [ -f "$MINI_TARBALL" ]
then
	rm -rf "$MINI_REPO" ||:
	tar xzf "$MINI_TARBALL" -C "$ROOT/_build"
	opam init --reinit offline_clone "$MINI_REPO" --no-setup
else
	opam init --reinit --no-setup
fi

eval "$(opam env)"
opam install \
	core_kernel.v0.11.1 \
	dune.1.6.3 \
	lwt.4.1.0 \
	lwt_log.1.1.0 \
	lwt_ppx.1.2.1 \
	pcre.7.4.0 \
	ppx_deriving.4.2.1 \
	visitors.20180513
