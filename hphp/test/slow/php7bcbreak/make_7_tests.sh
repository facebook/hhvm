#!/bin/bash
#
# Tiny little helper to create the empty php7 versions of your regression tests.
# See the README.

set -e
set -u

TMP_FILE=""

function abort() {
  echo "ERROR: $*" >&2
  exit 1
}

function cleanup() {
  if [ -f "$TMP_FILE" ]; then rm -f "$TMP_FILE"; fi
}

trap cleanup EXIT

function choose_temp_file() {
  local TMP=${TMP:-/tmp}
  local CANDIDATE
  local X

  [ -w "$TMP" ] || abort "Cannot write to $TMP (try setting \$TMP)"

  CANDIDATE="$TMP/hhvm_make_7_tests.$$.tmp"

  if [ ! -e "$CANDIDATE" ]; then
    touch "$CANDIDATE"
    TMP_FILE="$CANDIDATE"
    return 0
  fi

  for X in $(seq 1 10); do
    CANDIDATE="$TMP/hhvm_make_7_tests.$$.$X.tmp"
    if [ ! -e "$CANDIDATE" ]; then
      touch "$CANDIDATE"
      TMP_FILE="$CANDIDATE"
      return 0
    fi
  done

  abort "Somehow couldn't find a valid temp file"
}

choose_temp_file

pushd "$(dirname "$(readlink -f "$0")")" > /dev/null

function make_test() {
  local TEST_NAME=$1

  cat - > "$TMP_FILE" << HEREDOC
<?php

require_once __DIR__.'/../5/$TEST_NAME' ;
HEREDOC

  mv "$TMP_FILE" "../7/$TEST_NAME"
}

pushd ./5 > /dev/null

for FILE in *.php; do
  make_test "$FILE"
done
