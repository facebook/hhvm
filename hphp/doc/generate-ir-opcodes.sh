#!/bin/sh

# This fallback is for the cmake build, which won't have an FBCODE_DIR
# environment variable, and runs this from the runtime subdir.
DIR="$( cd "$( dirname "$0" )" && pwd )"
if [ x"$FBCODE_DIR" = x"" ] ; then
    FBCODE_DIR="$DIR/../.."
    INSTALL_DIR="$FBCODE_DIR/hphp/runtime"
fi

SCRIPT=$FBCODE_DIR/hphp/doc/generate-ir-opcodes.pl
SRC=$FBCODE_DIR/hphp/doc/ir.specification
OUTPUT=$INSTALL_DIR/ir-opcode-generated.h

perl $SCRIPT $SRC > $OUTPUT
