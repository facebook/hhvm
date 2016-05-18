#!/bin/sh
#
# Helper for running TestExt from an fbmake build.
#
# ./run_test_ext.sh
#

unset CDPATH

DIR="$( cd "$( dirname "$0" )" && pwd )"

# Tools don't agree on whether to pass relative or absolute paths, so accept
# both.
TEST_BIN="${DIR}/../../$1"
if [ ! -x "${TEST_BIN}" ]; then
  TEST_BIN="$1"
fi

cd "${DIR}/../" && "${TEST_BIN}" "$2" "$3" "$4"
