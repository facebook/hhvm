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

while :; do
    case $1 in
        --target-dir) TARGET_DIR=$2
          shift 2
        ;;
        --run) run=$1
          shift 1
        ;;

        *) break
    esac
done

if [ -z "${HACK_NO_CARGO_VENDOR}" ]; then
  LOCK_FLAG="--frozen"
else
  LOCK_FLAG="--locked"
fi

if [ -z "${TARGET_DIR}" ]; then
  TARGET_DIR="${HACK_BUILD_ROOT}/target"
fi

if [ -z ${HACKDEBUG+1} ]; then
  profile=release; profile_flags="--release"
else
  profile=debug; profile_flags=
fi

BUILD_PARAMS=()
BUILD_PARAMS+=("$LOCK_FLAG")
BUILD_PARAMS+=(--quiet)
BUILD_PARAMS+=(--target-dir "${TARGET_DIR}")
BUILD_PARAMS+=(--package "$pkg")
BUILD_PARAMS+=("$profile_flags")

( # add CARGO_BIN to PATH so that rustc and other tools can be invoked
  [[ -n "$CARGO_BIN" ]] && PATH="$CARGO_BIN:$PATH";
  # note: --manifest-path doesn't work with custom toolchain, so do cd
  cd "$HACK_SOURCE_ROOT" && \
  sed '/\/facebook\//d' ./.cargo/Cargo.toml.ocaml_build > ./Cargo.toml;
  if [ -z "$run" ]; then
    cargo build "${BUILD_PARAMS[@]}" && \
    cp "${TARGET_DIR}/$profile/lib$lib.a" "lib${lib}.a"
  else
    cargo run --bin "$lib" "$@"
  fi
)
