#!/usr/bin/env bash

HACKDIR=$(dirname "$0")
SKIP_OPAM=${SKIP_OPAM:-0}

cmake . -Wno-dev -DOCAML_EXECUTABLE="$(command -v ocaml)" \
  -DSKIP_OPAM="${SKIP_OPAM}"
make hack
hg rev "$HACKDIR/src/hhi/.merlin" "$HACKDIR/src/ppx/.merlin"
