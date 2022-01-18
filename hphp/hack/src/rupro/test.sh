#!/bin/bash

trap "exit" INT # exit on ctrl-c

function usage {
  echo "Usage:"
  echo "  buck run //hphp/hack/src/rupro:test -- [--vim] <FILEPATH>.php"
}

if [[ -z "$BUCK_DEFAULT_RUNTIME_RESOURCES" || \
      -z "$BUCK_PROJECT_ROOT" ]]; then
  usage >&2
  echo "fatal: use buck" >&2
  exit 1
fi
#BUCKOUT=$(echo "$BUCK_DEFAULT_RUNTIME_RESOURCES" | sed -E "s|(^.+buck-out).+$|\1|g")
#FBCODE=$(dirname "$BUCKOUT")
#HACK="$FBCODE/hphp/hack"

# Extract Buck mode
mode=$(grep -oP 'fbcode/buck-out/\K[^/]+' <<< "$BUCK_PROJECT_ROOT")
if [[ "$mode" != dev && "$mode" != opt ]]; then
  echo "fatal: unknown mode '$mode'" >&2
  exit 1
fi

# Locate binaries
HSTC_CAML="$BUCK_PROJECT_ROOT/buck-out/$mode/bin/hphp/hack/src/hh_single_type_check/hh_single_type_check.opt"
HSTC_RUST="$BUCK_PROJECT_ROOT/buck-out/$mode/bin/hphp/hack/src/rupro/stc/stc.opt"
[ -f "$HSTC_CAML" ] || { echo "fatal: ocaml hstc not found at $HSTC_CAML" >&2; exit 1; }
[ -f "$HSTC_RUST" ] || { echo "fatal: rust hstc not found at $HSTC_RUST" >&2; exit 1; }

f=""
vim=false
while (( "$#" )); do
  case "$1" in
    --vim) vim=true; shift;;
    *) f="$1"; shift;;
  esac
done

if [ -z "$f" ]; then
  usage >&2
  echo "fatal: must specify file" >&2
  exit 1
fi

caml_out=$(mktemp -t caml.XXXXXXXXX)
rust_out=$(mktemp -t rust.XXXXXXXXX)
"$HSTC_CAML" --no-builtins --tast "$f" &> "$caml_out" || true
"$HSTC_RUST" "$f" &> "$rust_out" || true

diff_result="$(diff -wbBdu --label ocaml --label rust <(cat "$caml_out") <(cat "$rust_out"))"

if [ -n "$diff_result" ]; then
  if [ "$vim" == true ]; then
    vim -d "$caml_out" "$rust_out"
  else
    cat <<<"$diff_result"
  fi
  exit 2
fi
echo "No differences"
exit 0
