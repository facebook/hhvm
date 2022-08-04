#!/bin/bash

trap "exit" INT # exit on ctrl-c

function usage {
  echo "Usage:"
  echo "  buck run //hphp/hack/src/hackrs:test -- [--vim] <FILEPATH>.php"
}

if [[ -z "$BUCK_DEFAULT_RUNTIME_RESOURCES" ]]; then
  usage >&2
  echo "fatal: use buck" >&2
  exit 1
fi

# Locate binaries
# In Buck v1 the BUCK_DEFAULT_RUNTIME_RESOURCES is target relative whereas
# in Buck v2 it is project (fbsource) relative. So for Buck v1, we have to
# "dirname" until we find the executables
RESOURCES_DIR="$BUCK_DEFAULT_RUNTIME_RESOURCES"
while true; do
  HSTC_CAML="$RESOURCES_DIR/hphp/hack/src/hh_single_type_check"
  [ -f "$HSTC_CAML" ] && break
  RESOURCES_DIR="$(dirname "$RESOURCES_DIR")"
done
HSTC_RUST="$RESOURCES_DIR/hphp/hack/src/hackrs/stc"
[ -f "$HSTC_CAML" ] || { echo "fatal: ocaml hstc not found anywhere around $BUCK_DEFAULT_RUNTIME_RESOURCES" >&2; exit 1; }
[ -f "$HSTC_RUST" ] || { echo "fatal: rust hstc not found anywhere around $BUCK_DEFAULT_RUNTIME_RESOURCES" >&2; exit 1; }

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
