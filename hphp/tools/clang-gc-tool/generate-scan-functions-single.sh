#!/bin/bash

# Helper script used to run clang on a single build log entry.

ROOT="$1"
shift
DIR="$1"
shift
OUTDIR="$1"
shift
PLUGIN_DLL="$1"
shift
VERBOSE="$1"
shift
COMPILE_ARGS=("$@")

if [[ $VERBOSE == "1" ]]; then
    PLUGIN_VERBOSE="-plugin-arg-add-scan-methods -v"
else
    PLUGIN_VERBOSE=""
fi

MORE_ARGS="-x c++ -fexceptions -fcxx-exceptions -Wno-unused-variable -Wno-inconsistent-missing-override -Wno-deprecated-declarations"

NUM_ARGS=${#COMPILE_ARGS[@]}
FILE=${COMPILE_ARGS[${NUM_ARGS} - 1]}
if [[ $FILE != "$DIR/"* ]]; then
    # ignore files outside specified directory.
    exit 0
fi
FULLNAME=$(basename "$FILE")
EXTENSION="${FULLNAME##*.}"
if [[ "$EXTENSION" != "cpp" && "$EXTENSION" != "cc" ]]; then
    # ignore everything but C++ files.
    exit 0
fi
CLANG=${COMPILE_ARGS[0]}
COMPILE_ARGS=("${COMPILE_ARGS[@]:1}")
mkdir -p "$OUTDIR"
NUM_ARGS=${#COMPILE_ARGS[@]}
COMPILE_ARGS[${NUM_ARGS} - 1]=$FILE
CLANG_CMD="$CLANG -cc1 -load $PLUGIN_DLL -plugin add-scan-methods -plugin-arg-add-scan-methods $OUTDIR $PLUGIN_VERBOSE $MORE_ARGS ${COMPILE_ARGS[*]}"
if [[ $VERBOSE == "1" ]]; then
    echo "$CLANG_CMD"
else
    echo -n "."
fi
#(cd "$ROOT" && $CLANG_CMD)
(cd "$ROOT" && $CLANG_CMD >> "$OUTDIR/warnings.log")
