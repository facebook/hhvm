#!/bin/bash

# Test input arguments:
#   $1 is path to the HHVM binary.
#   $2 is the path to thread_locals.txt.

# We use `nm` to list symbols, filter to thread locals in the HPHP:: namespace,
# use cut to select just the name, then strip trailing whitespace and ABI tags.
ALL=$( \
  nm --demangle --format sysv "$1" \
  | grep 'HPHP::' \
  | grep -v 'HPHP::HHBBC::' \
  | grep '\(\.tbss\|\.tdata\)' \
  | cut --delimiter '|' --field 1 \
  | sed -e 's/ *$//' \
  | sed -e 's/\[abi:[^]]*\]$//' \
)

# Remove bash-style comments from the whitelist file.
WHITELIST=$(sed -e 's/ *#.*//' "$2")

# comm -23 returns lines in the first input that are not in the second.
# We need -u becaues a symbol may appear multiple times with different ABI tags.
EXTRA=$(comm -23 <(echo "$ALL" | sort -u) <(echo "$WHITELIST" | sort -u))

if [ "$EXTRA" ]; then
  echo -e "
--------------------------------------------------------------------------------
\\e[0;31mERROR: New thread locals detected.\\e[0m
Per-request memory should be allocated using RDS locals to allow for
userland scheduling. New thread locals indicate a  potential logic error.
If these thread locals are necessary, add them to thread_locals.txt:

$EXTRA
--------------------------------------------------------------------------------
  "
  exit 1
fi
