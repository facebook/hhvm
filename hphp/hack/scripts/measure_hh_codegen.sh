#!/bin/sh

# This script compares codegen of hh against hhvm on some set of php files
# it does so by generating HHAS files with both hh and hhvm and comparing the
# results
#
# arguments: $1 directory containing php files (it is searched recursively)
# outputs: logs to scuba table hh_codegen_progress

if [ -z "$1" ]; then
  echo 'ERROR: expected argument'
  echo 'usage: measure_hh_codegen.sh <dir_of_php_files>'
  exit 1
fi
if [ "$(basename "$PWD")" != 'fbcode' ]; then
  echo "ERROR: wrong working directory $PWD"
  echo 'run this script from fbcode root directory'
  exit 1
fi

HHVM='/usr/local/hphpi/bin/hhvm'
PHP_FILES=$(find "$1"/ -name '*.php')
BUCK='tools/build/buck/bin/buck'

NUM_LINES_HHVM=0
NUM_LINES_HH=0
NUM_LINES_COMMON=0
for FILE in $PHP_FILES; do
  HHVM_TMP=$(mktemp /tmp/measure_hh_codegen.hhvm.XXXXXX)
  HH_TMP=$(mktemp /tmp/measure_hh_codegen.hh.XXXXXX)
  "$HHVM" -v Eval.DumpHhas=1 "$FILE" >"$HHVM_TMP"
  $BUCK run //hphp/hack/src:hh_single_compile -- "$FILE" >"$HH_TMP"
  NUM_LINES_HHVM=$((NUM_LINES_HHVM + $(wc -l < "$HHVM_TMP")))
  NUM_LINES_HH=$((NUM_LINES_HH + $(wc -l < "$HH_TMP")))
  NUM_LINES_COMMON=$((NUM_LINES_COMMON + $(/bin/diff -bBd --unchanged-group-format=\'%=\' --old-group-format=\'\' --new-group-format=\'\' --changed-group-format=\'\' "$HHVM_TMP" "$HH_TMP" | wc -l)))
  rm "$HHVM_TMP"
  rm "$HH_TMP"
done

# log to scuba
$BUCK run //hphp/hack/scripts:log_hh_codegen_results_to_scuba -- "$NUM_LINES_HHVM" "$NUM_LINES_HH" "$NUM_LINES_COMMON" "$1"
