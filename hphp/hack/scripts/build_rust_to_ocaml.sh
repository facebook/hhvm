#!/bin/bash
set -e
HACK_SOURCE_ROOT="${HACK_SOURCE_ROOT:-$HACKDIR}"
HACK_BUILD_ROOT="${HACK_BUILD_ROOT:-$HACKDIR}"

if [ -z "$HACK_SOURCE_ROOT" ]; then
   echo >&2 "ERROR: must set HACK_SOURCE_ROOT to point to hphp/hack source dir"
   exit 1
fi

if (( "$#" < 2 )); then
   echo "Usage: CARGO_BIN=path/to/cargo-bin $0 PACKAGE_NAME LIB_NAME"
   exit 2
fi
pkg="$1"
lib="$2"
shift 2

if [ -z "${HACK_NO_CARGO_VENDOR}" ]; then
  LOCK_FLAG="--frozen"
else
  LOCK_FLAG="--locked"
fi

TARGET_DIR="${HACK_BUILD_ROOT}/target"

profile=debug; profile_flags=
if [ -z ${HACKDEBUG+1} ]; then
  profile=release; profile_flags="--release"
fi
( # add CARGO_BIN to PATH so that rustc and other tools can be invoked
  [[ -n "$CARGO_BIN" ]] && PATH="$CARGO_BIN:$PATH";
  trap "[ -e ./Cargo.toml ] && rm ./Cargo.toml" EXIT
  # note: --manifest-path doesn't work with custom toolchain, so do cd
  cd "$HACK_SOURCE_ROOT" && \
  sed '/\/facebook\//d' ./.cargo/Cargo.toml.ocaml_build > ./Cargo.toml && \
  cargo build \
    $LOCK_FLAG \
    --quiet \
    --target-dir "${TARGET_DIR}" \
    --package "$pkg" \
    $profile_flags \
    "$@";
) && cp "${TARGET_DIR}/$profile/lib$lib.a" "lib${lib}_stubs.a"
