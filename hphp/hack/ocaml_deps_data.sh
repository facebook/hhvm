#!/bin/bash

export OCAML_VERSION=4.07.1
export OCAML_VARIANT=+fp

export HACK_OPAM_DEPS=(
  base.v0.11.1
  base64.2.2.0
  cmdliner.1.0.4
  core.v0.11.3
  core_kernel.v0.11.1
  dtoa.0.3.1
  dune.1.11.0
  fileutils.0.5.3
  lru.0.3.0
  lwt.4.2.1
  lwt_log.1.1.0
  lwt_ppx.1.2.2
  merlin.3.3.2
  ocp-indent.1.7.0
  ounit.2.2.1
  pcre.7.3.5
  ppx_deriving.4.2.1
  ppx_gen_rec.1.1.0
  sedlex.1.99.4
  sexplib.v0.11.0
  sqlite3.4.4.1
  uchar.0.0.2
  uutf.1.0.2
  visitors.20180513
  wtf8.1.0.1
  yojson.1.5.0
)

export HACK_OPAM_DOWNLOAD_ONLY_DEPS=""

# The rest of the file exports variables based on the above configuration.

export HACK_OCAML_VERSION="${OCAML_VERSION}${OCAML_VARIANT}"
if [ -n "$OCAML_VARIANT" ]; then
  export OCAML_BASE_NAME=ocaml-variants
  export OCAML_COMPILER_NAME="${OCAML_BASE_NAME}.${HACK_OCAML_VERSION}"

  # Both of these need to be cached in the mini-repo, but only one will be
  # installed per platform.
  export HACK_OPAM_DOWNLOAD_ONLY_DEPS=(
    "${HACK_OPAM_DOWNLOAD_ONLY_DEPS}"
    "ocaml-variants.${OCAML_VERSION}${OCAML_VARIANT}"
    "ocaml-base-compiler.${OCAML_VERSION}"
  )
else
  export OCAML_BASE_NAME="ocaml-base-compiler"
  export OCAML_COMPILER_NAME="${OCAML_BASE_NAME}.${HACK_OCAML_VERSION}"
fi

UNAME=$(uname -s)
if [ "$UNAME" != "Linux" ]; then
  # Some variants are not supported on other platforms, so we use the base
  # version instead.
  # +fp is known not to work on Macs, but other combinations have not been
  # tested.
  export HACK_OCAML_VERSION=${OCAML_VERSION}
fi
