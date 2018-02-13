#!/bin/bash

set -u
set -e

SUITE=$1
pushd "$(dirname "$(readlink -f "$0")")" > /dev/null

if [ ! -d "$SUITE" ]; then
  echo "Could not find directory $SUITE"
  exit 1
fi

if [ ! -d "tuple/tests" ]; then
  echo "Could not find original tests in tuple/tests"
  exit 1
fi

function link_expect() {
  local F=$1
  local N=$2
  local EXP=$3
  if [ -f "$F.$EXP" ]; then
    ln -sf "../../tuple/tests/$N.$EXP" "$SUITE/tests/$N.$EXP"
  else
    rm -f "$SUITE/tests/$N.$EXP"
  fi
}

mkdir -p "$SUITE/tests"

gen="generated"

for FILE in tuple/tests/*.php; do
  NAME=$(basename "$FILE")
  DST_FILE="$SUITE/tests/$NAME"
  sed "s/<?\(hh\|php\)/<?\1 \/\/ @$gen by make_suite.sh/" $FILE > $DST_FILE
  link_expect "$FILE" "$NAME" "expect"
  link_expect "$FILE" "$NAME" "expectf"
  link_expect "$FILE" "$NAME" "expectregex"
  link_expect "$FILE" "$NAME" "opts"
  link_expect "$FILE" "$NAME" "hphp_opts"
  link_expect "$FILE" "$NAME" "skipif"
done
