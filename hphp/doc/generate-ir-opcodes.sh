#!/bin/bash

function die {
  echo $1 1>&2
  exit 1
}

# This fallback is for the cmake build, which won't have an INSTALL_DIR
# environment variable, and runs this from the runtime subdir.
if [ x"$INSTALL_DIR" = x"" ] ; then
    cd "$(dirname "$(which "$0")")" || die "Can't find script dir for $0" 1>&2
    INSTALL_DIR="$(pwd)/../runtime"
fi

SCRIPT=generate-ir-opcodes.pl
SRC=ir.specification
OUTPUT=$INSTALL_DIR/ir-opcode-generated.h

perl $SCRIPT $SRC > $OUTPUT
