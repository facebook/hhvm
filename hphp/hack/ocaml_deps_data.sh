#!/bin/bash

export OCAML_VERSION="5.2.0+options"

export HACK_OPAM_DEPS=(
  base.v0.16.3
  base64.3.5.0
  camlp-streams.5.0.1
  cmdliner.1.1.1
  core_kernel.v0.16.0
  core_unix.v0.16.0
  dtoa.0.3.2
  dune.3.6.0
  fileutils.0.6.4
  fmt.0.9.0
  iomux.0.3
  landmarks-ppx.1.4
  lru.0.3.1
  lwt.5.7.0
  lwt_log.1.1.2
  lwt_ppx.2.1.0
  memtrace.0.2.3
  merlin.5.0-502
  mtime.1.4.0
  ocp-indent.1.8.1
  ounit2.2.2.6
  pcre.7.5.0
  ppx_deriving.5.2.1
  ppx_gen_rec.2.0.0
  ppx_sexp_conv.v0.16.0
  ppx_yojson_conv.v0.16.0
  sedlex.3.0
  sexplib.v0.16.0
  sqlite3.5.1.0
  uchar.0.0.2
  uutf.1.0.3
  visitors.20210608
  wtf8.1.0.2
  yojson.2.0.2
  ocamlbuild.0.14.3
  ocaml-option-flambda
  ocaml-option-no-compression
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
