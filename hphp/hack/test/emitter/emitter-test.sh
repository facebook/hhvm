#!/bin/bash

TEST="$1"
EMITTER="$2"
HHVM="$3"

HHVM_CMD="$HHVM -vRepo.Local.Mode=-- -vRepo.Central.Path=verify.hhbc"

HHAS="$TEST.hhas"
EXPECTED="$TEST.exp"

# First, compute the expected output using regular HHVM execution
(cat "$TEST"; echo "test();") | $HHVM_CMD /dev/stdin > "$EXPECTED"

# Bytecode compile with the emitter and run the generated assembly
"$EMITTER" --emit "$TEST" > "$HHAS" &&
  $HHVM_CMD -vEval.AllowHhas=1 "$HHAS"
