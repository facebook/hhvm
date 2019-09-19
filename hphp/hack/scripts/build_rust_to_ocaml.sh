#!/bin/bash
set -e
HACK_SOURCE_ROOT=${HACK_SOURCE_ROOT:-$HACKDIR}

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

profile=debug; profile_flags=
if [ -z ${HACKDEBUG+1} ]; then
  profile=release; profile_flags="--release"
fi
( # add CARGO_BIN to PATH so that rustc and other tools can be invoked
  [[ -n "$CARGO_BIN" ]] && PATH="$CARGO_BIN:$PATH";
  # note: --manifest-path doesn't work with custom toolchain, so do cd
  cd "$HACK_SOURCE_ROOT" && \
  cargo build $LOCK_FLAG --package "$pkg" $profile_flags "$@"
) && cp "$HACK_SOURCE_ROOT/target/$profile/lib$lib.a" "lib${lib}_stubs.a"
