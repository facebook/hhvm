#! /usr/bin/env bash

# Usage: First do 'make -C ../src test' to generate the .out files. If there are
# failures, their test filenames will be printed to stdout. Pass these as
# arguments to this script.

# note: we don't use [ -z $FOO ] to check if FOO is unset because that is also
# true if FOO=""
if [ -z "${OUT_EXT+x}" ]; then
  OUT_EXT=".out"
fi

if [ -z "${EXP_EXT+x}" ]; then
  EXP_EXT=".exp"
fi

DIFF=`command -v colordiff` || diff

for f in $@; do
  nl --body-numbering=a $f
  if [ -e "$f$EXP_EXT" ]; then
    EXP="$f$EXP_EXT"
  else
    EXP=/dev/null
  fi
  cat $EXP
  $DIFF $EXP "$f$OUT_EXT"
  read -p "Copy output to expected output? (y|n|q)" -n 1 -r
  echo ""
  if [ "$REPLY" = "y" ]; then
    cp "$f$OUT_EXT" "$f$EXP_EXT"
  elif [ "$REPLY" = "q" ]; then
    exit 0
  fi
done
