#!/bin/bash

export OCAML_VERSION="4.12.1+options"

export HACK_OPAM_DEPS=(
  base.v0.14.1
  base64.3.5.0
  camlp4.4.12+1
  cmdliner.1.0.4
  core_kernel.v0.14.1
  dtoa.0.3.2
  dune.2.9.0
  fileutils.0.6.3
  fmt.0.8.9
  landmarks-ppx.1.4
  lru.0.3.0
  lwt.5.4.0
  lwt_log.1.1.1
  lwt_ppx.2.0.2
  merlin.4.3.1-412
  mtime.1.2.0
  ocp-indent.1.8.1
  ounit2.2.2.4
  pcre.7.4.6
  ppx_deriving.5.2.1
  ppx_gen_rec.2.0.0
  sedlex.2.3
  sexplib.v0.14.0
  sqlite3.5.0.2
  uchar.0.0.2
  uutf.1.0.2
  visitors.20210316
  wtf8.1.0.2
  yojson.1.7.0
  ocaml-option-flambda
)

# The rest of the file exports variables based on the above configuration.

export HACK_OCAML_VERSION="${OCAML_VERSION}"
export OCAML_BASE_NAME=ocaml-variants
export OCAML_COMPILER_NAME="${OCAML_BASE_NAME}.${HACK_OCAML_VERSION}"

UNAME=$(uname -s)
if [ "$UNAME" != "Linux" ]; then
  # Some variants are not supported on other platforms, so we use the base
  # version instead.
  # +fp is known not to work on Macs, but other combinations have not been
  # tested.
  echo 'Non linux platform detected, skipping +fp'
else
  HACK_OPAM_DEPS+=(ocaml-option-fp)
  export HACK_OPAM_DEPS
fi
