#!/bin/bash
hack_root="${HACKDIR}"

if [ -z "$HACKDIR" ]; then
   echo >&2 "ERROR: must set HACKDIR to point to hphp/hack source dir"
   exit 1
fi

if (( "$#" < 2 )); then
   echo "Usage: CARGO_BIN=path/to/cargo-bin $0 PACKAGE_NAME LIB_NAME"
   exit 2
fi
pkg="$1"
lib="$2"
shift 2

profile=debug; profile_flags=
if [ -z ${HACKDEBUG+1} ]; then
  profile=release; profile_flags="--release"
fi
( # add CARGO_BIN to PATH so that rustc and other tools can be invoked
  [[ -n "$CARGO_BIN" ]] && PATH="$CARGO_BIN:$PATH";
  # note: --manifest-path doesn't work with custom toolchain, so do cd
  cd "$hack_root" && \
  cargo build --frozen --package "$pkg" $profile_flags "$@"
) && cp "$hack_root/target/$profile/lib$lib.a" "lib${lib}_stubs.a"
