#!/bin/bash
set -e # terminate upon non-zero-exit-codes (in case of pipe, only checks at end of pipe)

# This helper script is for launching naming_table_builder.
# It must be launched with the following arguments in order:
# --fbcode_dir=.. the path to root of fbcode (no trailing /)
# --install_dir=... where the output sql file should go (no trailing /)
# --output=.. the name of the output sql file
# --www=... the relative path within fbcode (no leading or trailing /) of the repository
# --builder=.. path to naming_table_builder binary

FBCODE_DIR=${1/#--fbcode_dir=/}
INSTALL_DIR=${2/#--install_dir=/}
OUTPUT=${3/#--output=/}
WWW=${4/#--www=/}
BUILDER=${5/#--builder=/}
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry

"$BUILDER" --www "$FBCODE_DIR/$WWW" --output "$INSTALL_DIR/$OUTPUT"
