#!/usr/bin/env bash

OTHER_ARGS=()

while (( "$#" )); do
  case "$1" in
    --hhstc-path) HHSTC=$2; shift 2;;
    --hh-quickfix-lint-path) QUICKFIX_LINT=$2; shift 2;;
    *) OTHER_ARGS+=("$1"); shift;;
  esac
done

set -- "${OTHER_ARGS[@]}"

FILE=$1

"$HHSTC" --lint-json "$FILE" | "$QUICKFIX_LINT"
