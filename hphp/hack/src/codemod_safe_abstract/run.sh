#!/bin/bash
set -e
set -o pipefail
set -x
# # Codemod for seeing how many `<<__NeedsConcrete>>`s we will need to add
#
# A "what-if" analysis that modifies all-WWW locally.
# It adds the attribute and runs `hh` in a loop until there are no more errors.
# Note that it adds a few temporary commits called 'Iterate' as it goes, so you
# can see what changed in each iteration and how many iterations there were.
#
# # env vars:
# ROOT controls repo root, defaults to pwd
# FBCODE controls where we look for fbcode, defaults to ~/fbsource/fbcode
# prereqs:
# - `hh` should work in $ROOT. Depending on what and where you're running,
# you may have to do `hh --ignore-hh-version` first or `hh --no-load`

root=${ROOT:-$(pwd)}
fbcode=${FBCODE:-"~/fbsource/fbcode"}
mode=@//mode/opt-clang

round=0

orig_dir=$(pwd)

fail() {
    # arg1 message
    # arg1 code
    cd "$orig_dir"
    echo "$1" 1>&2
    exit "$2"
}

run() {
  cd "$root"
  if [ -n "$(sl st)" ]; then
    fail "working directory must be clean" 2
  fi
  local commit
  commit=$(sl id)
  local errors="/tmp/codemod-sa-$commit-$round-errors"
  local out="/tmp/codemod-sa-$commit-$round-out"
  hh --re --root "$root" --json --config needs_concrete=true > "$out" \
    || [ $? -eq 2 ] # hh_distc uses retrun code of 2 if there are errors
  grep '{"errors"' "$out" > "$errors"
  cd "$fbcode"
  buck2 run "$mode" //hphp/hack/src/codemod_safe_abstract:codemod_safe_abstract -- \
    --errors "$errors" \
    --root "$root" || fail "codemod failed" 3
  cd -
  echo "round $round"
}

run

while [ -n "$(sl st)" ]; do
  hg commit -m "safe_abstract_codemod round $round"
  ((round++)) || true
  run
done
echo 'done'
