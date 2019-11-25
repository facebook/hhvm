#!/bin/bash

hhvm="$1"
cmds="$2"
gdblib="$3"
hhas="$4"
expect="$5"
is_clang="$6"

if [[ $is_clang = "1" ]]; then
    exit 0
fi

log="$(mktemp)"

trap 'rm "$log"' EXIT

set -e

gdb --nx -ex "source $gdblib" -ex "set log file $log" -x "$cmds" --args "$hhvm" -vEval.Jit=false -vEval.AllowHhas=true "$hhas"

sed -i -e 's/^0x[0-9a-fA-F]*+//' "$log"

if diff -w "$expect" "$log"; then
    exit 0
fi

echo "ERROR: The hhx command produced incorrect output for '$hhas'"
echo "If you changed the bytecode encoding, please fix hhbc.py to match."
echo "If you changed '$hhas', please update '$expect' to match the new format:"

cat "$log"
exit 1
