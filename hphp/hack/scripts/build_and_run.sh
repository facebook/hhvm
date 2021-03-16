#!/bin/bash
# Copyright (c) 2015, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.

set -e
set -x

SCRIPT_DIR="$(dirname "$0")"
FBCODE_ROOT="$(realpath "${SCRIPT_DIR}/../../../")"
HPHP_ROOT="${FBCODE_ROOT}/hphp"
HACK_ROOT="${FBCODE_ROOT}/hphp/hack"
FB_DUNE_BUILD_DIR="${HACK_ROOT}/facebook/redirect/dune_build"

HACK_SUBDIR="$1"
TARGET="$2"

ARGS=($@)
ARGS=(${ARGS[@]:2})

function dune_build() {
  # OCaml
  if [ -e "${HACK_ROOT}/${HACK_SUBDIR}/${TARGET}.ml" ]; then
    (
      cd "${HACK_ROOT}"
      "${DUNE}" build "${HACK_SUBDIR}/${TARGET}.exe"
    )
    exec "${DUNE_BUILD_DIR}/default/$HACK_SUBDIR/${TARGET}.exe" "${ARGS[@]}"
  fi

  # Rust
  if [ -e "${HACK_ROOT}/${HACK_SUBDIR}/${TARGET}.rs" ]; then
    export CARGO_TARGET_DIR="${DUNE_BUILD_DIR}/cargo"
    (
      cp "${HACK_ROOT}/.cargo/Cargo.toml.ocaml_build" "${HACK_ROOT}/Cargo.toml"
      cd "${HACK_ROOT}/${HACK_SUBDIR}"
      "${CARGO}" build --bin "${TARGET}"
      [ -e "${HACK_ROOT}/Cargo.toml" ] && rm "${HACK_ROOT}/Cargo.toml"
    )
    exec "${CARGO_TARGET_DIR}/debug/${TARGET}" "${ARGS[@]}"
  fi
}

cd "${FBCODE_ROOT}"
if [ -e "${HACK_ROOT}/facebook/dune.sh" ] && [ -e "${FB_DUNE_BUILD_DIR}" ]; then
  # FB Dune
  DUNE="${HACK_ROOT}/facebook/dune.sh" \
  CARGO="${HACK_ROOT}/scripts/facebook/cargo.sh" \
  DUNE_BUILD_DIR="${FB_DUNE_BUILD_DIR}"
  dune_build
elif [ -e "${FBCODE_ROOT}/third-party/CMakeLists.txt" ]; then
  # Open Source Dune
  DUNE="dune" \
  CARGO="cargo" \
  dune_build
elif [ -e "${HPHP_ROOT}/facebook" ]; then
  # FB Buck
  exec buck run "//hphp/hack/${HACK_SUBDIR}:${TARGET}" -- "${ARGS[@]}"
else
  echo "Couldn't determine build system"
  exit 1
fi
