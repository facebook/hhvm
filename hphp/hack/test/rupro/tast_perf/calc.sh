#!/bin/bash

set -euo pipefail

SCRIPT_DIR=$(dirname "$0")
TAST_DIR="$SCRIPT_DIR"
RESULTS="$SCRIPT_DIR/results.txt"
HG_ROOT=$(hg root)
N=10000

# Build both hh_single_type_check and the Rust stc
set -x
buck2 build @//mode/opt-clang //hphp/hack/src:hh_single_type_check //hphp/hack/src/rupro:stc
ML_STC="$HG_ROOT"/$(buck2 targets @//mode/opt-clang //hphp/hack/src:hh_single_type_check --show-output | awk '{print $2}')
RS_STC="$HG_ROOT"/$(buck2 targets @//mode/opt-clang //hphp/hack/src/rupro:stc --show-output | awk '{print $2}')
set +x

# For each file do a 1000x type check and write to the results.txt file
echo "file,ocaml,rust_no_pos,pct_change_no_pos,rust_with_pos,pct_change_with_pos" > "$RESULTS"
for php_file in "$TAST_DIR"/*.php; do
  echo "[+] Processing $php_file" >&2
  ML_TIME=$("$ML_STC" --profile-type-check-multi "$N" --no-builtins --skip-hierarchy-checks --skip-tast-checks "$php_file" | grep "total warm cpu time" | awk '{print $NF}')
  RS_TIME_NO_POS=$("$RS_STC" --profile-type-check-multi "$N" "$php_file" | grep "total warm cpu time" | awk '{print $NF}')
  RS_TIME_WITH_POS=$("$RS_STC" --with-pos --profile-type-check-multi "$N" "$php_file" | grep "total warm cpu time" | awk '{print $NF}')
  PCT_CHANGE_NO_POS=$(echo "$RS_TIME_NO_POS * 100 / $ML_TIME - 100" | bc -l)
  PCT_CHANGE_NO_POS=$(printf "%.0f" "$PCT_CHANGE_NO_POS")
  PCT_CHANGE_WITH_POS=$(echo "$RS_TIME_WITH_POS * 100 / $ML_TIME - 100" | bc -l)
  PCT_CHANGE_WITH_POS=$(printf "%.0f" "$PCT_CHANGE_WITH_POS")
  echo "$(basename "$php_file"),$ML_TIME,$RS_TIME_NO_POS,${PCT_CHANGE_NO_POS}%,$RS_TIME_WITH_POS,${PCT_CHANGE_WITH_POS}%" >> "$RESULTS"
done
