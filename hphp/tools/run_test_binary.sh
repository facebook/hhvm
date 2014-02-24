#!/bin/sh
#
# Helper for running TestExt from an fbmake build.
#
# ./run_test_ext.sh
#
HPHP_HOME=$(git rev-parse --show-toplevel)
: ${FBMAKE_BIN_ROOT=_bin}
cd $HPHP_HOME/hphp && $HPHP_HOME/$1 "$2" "$3" "$4"
