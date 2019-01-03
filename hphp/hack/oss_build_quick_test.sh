#!/usr/bin/env bash

HACKDIR="$(realpath "$(dirname "$0")")"
SKIP_OPAM=${SKIP_OPAM:-0}

cmake "$HACKDIR" -Wno-dev -DOCAML_EXECUTABLE="$(command -v ocaml)" \
  -DSKIP_OPAM="${SKIP_OPAM}" -DWITH_DUNE=1
cd "$HACKDIR" && make hack_dune
hg rev "$HACKDIR/src/hhi/.merlin" "$HACKDIR/src/ppx/.merlin"
