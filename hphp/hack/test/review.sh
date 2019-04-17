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

if [ -z "${NO_COPY+x}" ]; then
  NO_COPY=false
fi

ARROW="$(tput bold)$(tput setaf 6)==>$(tput setaf 7)"

for f in "$@"; do
  echo "$ARROW $f $(tput sgr0)"
  # `-b a` means to number all lines; this is the same as
  # --body-numbering=a, but works with both BSD and GNU `nl`
  nl -b a "$f"
  echo
  if [ -e "$f$EXP_EXT" ]; then
    EXP="$f$EXP_EXT"
  elif [ -n "${FALLBACK_EXP_EXT+x}" ] && [ -e "$f$FALLBACK_EXP_EXT" ]; then
    EXP="$f$FALLBACK_EXP_EXT"
  else
    EXP=/dev/null
  fi

  if [ -n "${OUTPUT_ROOT}" ]; then
    if [[ "$f" = "$SOURCE_ROOT"* ]]; then
      f="${OUTPUT_ROOT}${f#"${SOURCE_ROOT}"}"
    elif [[ "$f" = ./hphp/hack/* ]]; then
      f="${OUTPUT_ROOT}/${f#"./hphp/hack/"}"
    elif [[ "$f" = hphp/hack/* ]]; then
      f="${OUTPUT_ROOT}/${f#"hphp/hack/"}"
    fi
  fi

  if [ -e "$f$OUT_EXT" ]; then
    OUT="$f$OUT_EXT"
  elif [ -n "${FALLBACK_OUT_EXT+x}" ] && [ -e "$f$FALLBACK_OUT_EXT" ]; then
    OUT="$f$FALLBACK_OUT_EXT"
  else
    OUT=/dev/null
  fi
  echo "$ARROW Diff between $EXP and $(basename "$OUT") $(tput sgr0)"

  # Use git diff to give us color and word diffs. The patience algorithm
  # produces more readable diffs in some situations.
  #
  # --no-index makes us ignore the git repo, if any - otherwise this only
  # works in hg checkouts (i.e. fbcode)
  git --no-pager diff --no-index --diff-algorithm=histogram --color=always \
    --word-diff=color --word-diff-regex='[a-zA-Z0-9_:;-]+' \
    $EXP "$OUT" | tail -n +5
  echo
  if [ "$NO_COPY" = true ]; then
    if [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)View next file? [Y/q] $(tput sgr0)"
    else
      read -p "$(tput bold)View next file? [Y/q] $(tput sgr0)" -n 1 -r
    fi
  elif [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)"
  else
      read -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)" -n 1 -r
  fi
  echo ""
  if [ "$REPLY" = "y" ] && [ "$NO_COPY" = false ]; then
    cp "$OUT" "$f$EXP_EXT"
  elif [ "$REPLY" = "q" ]; then
    exit 0
  fi

  # A single blank line between loop iterations, even if the user hit enter.
  if [[ "$REPLY" =~ [a-zA-Z] ]]; then
    echo
  fi
done
