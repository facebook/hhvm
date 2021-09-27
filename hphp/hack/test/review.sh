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

if [ -z "${UPDATE+x}" ]; then
  # "always", "prompt" or "never"
  UPDATE=prompt
fi

ARROW="$(tput bold)$(tput setaf 6)==>$(tput setaf 7)"

function reconstitute_full_path {
  TEST_PATH=$1
  ROOT=$2
  if [ -n "${ROOT}" ]; then
    if [[ "$TEST_PATH" = "$SOURCE_ROOT"* ]]; then
      FULL_PATH="${ROOT}${TEST_PATH#"${SOURCE_ROOT}"}"
    elif [[ "$TEST_PATH" = ./hphp/hack/* ]]; then
      FULL_PATH="${ROOT}/${TEST_PATH#"./hphp/hack/"}"
    elif [[ "$TEST_PATH" = hphp/hack/* ]]; then
      FULL_PATH="${ROOT}/${TEST_PATH#"hphp/hack/"}"
    fi
  fi
  echo "$FULL_PATH"
}

for f in "$@"; do
  echo "$ARROW $f $(tput sgr0)"
  # `-b a` means to number all lines; this is the same as
  # --body-numbering=a, but works with both BSD and GNU `nl`
  nl -b a "$(reconstitute_full_path "$f" "$SOURCE_ROOT")"
  echo

  if [ "$VERIFY_PESSIMISATION" = true ]; then
    NAME_PESS=$f".pess"
    if [ -f "$NAME_PESS" ]; then
      echo "$(tput setaf 6)[pessimised into]$(tput setaf 7)$ARROW $NAME_PESS $(tput sgr0)"
      nl -b a "$(reconstitute_full_path "$NAME_PESS" "$SOURCE_ROOT")"
    fi
  fi

  OUT_BASE=$(reconstitute_full_path "$f" "$OUTPUT_ROOT")
  EXP_BASE=$(reconstitute_full_path "$f" "$SOURCE_ROOT")
  if [ -e "$OUT_BASE$OUT_EXT" ]; then
    OUT="$OUT_BASE$OUT_EXT"
  else
    OUT=/dev/null
  fi
  EXP_TO="$EXP_BASE$EXP_EXT"
  if [ -e "$EXP_BASE$EXP_EXT" ]; then
    EXP="$EXP_BASE$EXP_EXT"
  elif [ -n "${FALLBACK_EXP_EXT+x}" ] && [ -e "$EXP_BASE$FALLBACK_EXP_EXT" ]; then
    EXP="$EXP_BASE$FALLBACK_EXP_EXT"
  else
    EXP=/dev/null
  fi

  echo "$ARROW Diff between $EXP and $(basename "$OUT") $(tput sgr0)"

  # Use git diff to give us color and word diffs. The patience algorithm
  # produces more readable diffs in some situations.
  #
  # --no-index makes us ignore the git repo, if any - otherwise this only
  # works in hg checkouts (i.e. fbcode)
  git --no-pager diff --no-index --diff-algorithm=histogram --color=always \
    --word-diff=color --word-diff-regex='[^[:space:]]+' \
    $EXP "$OUT" | tail -n +5
  echo

  if [ "$UPDATE" = always ]; then
    # UPDATE=always updates all the .exp files without prompting.
    cp "$OUT" "$EXP_TO"
  elif [ "$UPDATE" = never ]; then
      # UPDATE=never allows users to see a diff of each .exp file with .out.
    if [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)View next file? [Y/q] $(tput sgr0)"
    else
      read -p "$(tput bold)View next file? [Y/q] $(tput sgr0)" -n 1 -r
    fi

    if [ "$REPLY" = "q" ]; then
      exit 0
    fi
  else
    # UPDATE=prompt allows users to see a diff of each .exp with
    # .out, then prompts to replace the .exp file.
    if [ "$TERM" = "dumb" ]; then
      read -r -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)"
    else
      read -p "$(tput bold)Copy output to expected output? [y/q/N] $(tput sgr0)" -n 1 -r
    fi

    if [ "$REPLY" = "y" ]; then
      cp "$OUT" "$EXP_TO"
    elif [ "$REPLY" = "q" ]; then
      exit 0
    fi
  fi

  # A single blank line between loop iterations, even if the user hit enter.
  if [[ "$REPLY" =~ [a-zA-Z] ]]; then
    echo
  fi
done
