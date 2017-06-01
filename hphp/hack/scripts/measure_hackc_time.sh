#!/bin/bash

# This script measures the time to compile a given directory with the hack
# compiler

if [ -z "$1" ]; then
  echo 'ERROR: expected argument'
  echo 'usage: measure_hackc_time.sh <dir_of_php_files>'
  exit 1
fi
if [ "$(basename "$PWD")" != 'fbcode' ]; then
  echo "ERROR: wrong working directory $PWD"
  echo 'run this script from fbcode root directory'
  exit 1
fi

BUCK='tools/build/buck/bin/buck'
HACKC='hphp/hack/scripts/hackc.py'

# prepare hackc
"$BUCK" build //hphp/hack/src:hh_single_compile 2> /dev/null
HACKC_PATH=$("$BUCK" targets --show-output //hphp/hack/src:hh_single_compile 2> /dev/null | cut -d' ' -f2)

# do the actual compilation
RESULT_IN_SEC=$("$HACKC" --path "$HACKC_PATH" --quiet "$1")

# TODO: log to scuba

echo "$RESULT_IN_SEC"
