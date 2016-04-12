#!/bin/sh
#
# Helper for running TestExt from an fbmake build.
#
# ./run_test_ext.sh
#

DIR="$( cd "$( dirname "$0" )" && pwd )"
cd $DIR/../ && $DIR/../../$1 "$2" "$3" "$4"
