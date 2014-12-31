#! /usr/bin/env bash

# Usage: First do 'make -C ../src test' to generate the .out files. If there are
# failures, their test filenames will be printed to stdout. Pass these as
# arguments to this script.

DIFF=`command -v colordiff` || diff

for f in $@; do
  cat $f
  if [ -e "$f.exp" ]; then
    EXP="$f.exp"
  else
    EXP=/dev/null
  fi
  $DIFF $EXP "$f.out"
  read -p "Copy .out to .exp? (y|n)" -n 1 -r
  if [ "$REPLY" = "y" ]; then
    cp "$f.out" "$f.exp"
  fi
done
