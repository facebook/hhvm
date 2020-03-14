#!/bin/bash

hhvm="$1"
cmds="$2"
gdblib="$3"
hhas="$4"
expect="$5"
is_clang="$6"
hhx="$7"
extra=("${@:8}")

if [[ $is_clang = "1" ]]; then
    exit 0
fi

log="$(mktemp)"

trap 'rm "$log"' EXIT

set -e

gdb --nx -ex "source $gdblib" -ex "set log file $log" -x "$cmds" --args "$hhvm" -vEval.Jit=false -vEval.AllowHhas=true "${extra[@]}" "$hhas"

sed -i -e 's/0x[0-9a-fA-F]*/0xXX/g' -e 's/\.cpp:[0-9]\+/.cpp/g' -e 's%at [^ ]*/fbcode/hphp/%at hphp/%g' "$log"

if diff -w "$expect" "$log"; then
    exit 0
fi

if [[ -n $hhx ]] ; then
    echo "ERROR: The hhx command produced incorrect output for '$hhas'"
    echo "If you changed the bytecode encoding, please fix hhbc.py to match."
    echo "If you changed '$hhas', please update '$expect' to match the new format:"
else
    echo "Test failed. Full output follows:"
    echo
fi

cat "$log"
exit 1
