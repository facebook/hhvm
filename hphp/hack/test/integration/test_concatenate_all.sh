#!/bin/bash
# Copyright (c) Facebook, Inc. and its affiliates.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
set -e

WORKING_DIR="$(pwd)"
TEST_DIR="$(realpath "$(dirname "$0")")"
DATA_DIR="${TEST_DIR}/data/concat_all"

if [ ! -d "${DATA_DIR}" ]; then
  DATA_DIR="${WORKING_DIR}/data/concat_all"
fi
if [ ! -d "${DATA_DIR}" ]; then
  DATA_DIR="${WORKING_DIR}/hphp/hack/test/integration/data/concat_all"
fi

if [ ! -d "${DATA_DIR}" ]; then
  echo "Couldn't find data directory: $DATA_DIR"
  exit 1
fi

function usage() {
  echo "Usage: $0 /path/to/hh_client /path/to/hh_server"
}

HH_CLIENT="$1"
if [ ! -e "${HH_CLIENT}" ]; then
  usage
  exit 1
fi

HH_SERVER="$2"
if [ ! -e "${HH_SERVER}" ]; then
  usage
  exit 1
fi
HH_SERVER_DIR="$(dirname "${HH_SERVER}")"
if [ "${HH_SERVER}" != "${HH_SERVER_DIR}/hh_server" ]; then
  # We don't have a way to actually specify the executable, so
  # we set $PATH instead
  echo "${HH_SERVER} must be an executable called hh_server"
  exit 1
fi
export PATH="${HH_SERVER_DIR}:$PATH"
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry

NEW_DATA_DIR="$(mktemp -d)"
cp -Ra "${DATA_DIR}"/{*.hack,*.php,.hhconfig} "${NEW_DATA_DIR}/"
OUTPUT_DIR="${DATA_DIR}"
DATA_DIR="${NEW_DATA_DIR}"

function cleanup() {
  cd "${DATA_DIR}"
  "${HH_CLIENT}" stop
  cd /
  rm -rf --preserve-root "${DATA_DIR}"
}
trap cleanup exit

cd "${DATA_DIR}"
"${HH_CLIENT}" stop || true
"${HH_CLIENT}"

# syntax tests
"${HH_CLIENT}" --concatenate-all no_ns*.hack > "${OUTPUT_DIR}/no_ns.hack.out"
"${HH_CLIENT}" --concatenate-all ns_body*.hack > "${OUTPUT_DIR}/ns_body.hack.out"
"${HH_CLIENT}" --concatenate-all ns_empty_body*.{hack,php} > "${OUTPUT_DIR}/ns_empty_body.hack.out"

# 01-child, 02-parent, 03-grandparent are intentionally in the wrong order; we
# need to check that concatenate-all reorders them
# 04-sibling is present to make sure the dep table is actually used instead of
# merely reversing the import order
"${HH_CLIENT}" --concatenate-all 0{1,2,3,4}*.hack > "${OUTPUT_DIR}/dependencies.hack.out"

cd "${OUTPUT_DIR}"
OUTPUT_DIR_RELATIVE="$(realpath --relative-to="${WORKING_DIR}" "${OUTPUT_DIR}")"
EXIT_CODE=0
for OUT in *.out; do
  EXPECT="$(basename "$OUT" .out).exp"
  if [ ! -e "$EXPECT" ]; then
    echo ">>> NEW TEST: ${OUT} <<<"
    sed 's/^/ >  /' "$OUT"
    echo "Would you like to record ${EXPECT} ? y/n"
    read -r RESPONSE
    if [ "$RESPONSE" = "y" ]; then
      cp "$OUT" "$EXPECT"
    else
      EXIT_CODE=1
    fi
  elif ! diff "$OUT" "$EXPECT" > /dev/null; then
    # diff exits with 0 if files are the same, 1 if there are differences
    echo ">>> Error: ${OUT} does not match ${EXPECT}:"
    git diff --no-index --color=always --word-diff=color \
      --src-prefix="${OUTPUT_DIR_RELATIVE}/" \
      --dst-prefix="${OUTPUT_DIR_RELATIVE}/" \
      "$EXPECT" "$OUT"
    EXIT_CODE=1
  fi
done

exit $EXIT_CODE
