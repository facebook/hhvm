#!/bin/bash

# This script compares codegen of hh against hhvm on some set of php files
# it does so by generating HHAS files with both hh and hhvm and comparing the
# results
#
# arguments: $1 directory containing php files (it is searched recursively)
# outputs: logs to scuba table hh_codegen_progress

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

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
DIFF_TMP=$(mktemp /tmp/diff_hh_codegen.XXXXXX)

NUM_LINES_HHVM=0
NUM_LINES_HH=0
NUM_LINES_COMMON=0
for FILE in $PHP_FILES; do
  HHVM_TMP=$(mktemp /tmp/measure_hh_codegen.hhvm.XXXXXX)
  HH_TMP=$(mktemp /tmp/measure_hh_codegen.hh.XXXXXX)
  "$HHVM" -m dumphhas -v Eval.AllowHhas=1  -v Eval.EnableHipHopSyntax=1  "$FILE" > "$HHVM_TMP"
  $BUCK run //hphp/hack/src:hh_single_compile -- "$FILE" > "$HH_TMP"
  NUM_LINES_HHVM=$((NUM_LINES_HHVM + $(wc -l < "$HHVM_TMP")))
  NUM_LINES_HH=$((NUM_LINES_HH + $(wc -l < "$HH_TMP")))
  NUM_LINES_COMMON=$((NUM_LINES_COMMON + $(/bin/diff -bBd --unchanged-group-format=\'%=\' --old-group-format=\'\' --new-group-format=\'\' --changed-group-format=\'\' "$HHVM_TMP" "$HH_TMP" | wc -l)))
  { echo "*******************************************************";
    echo "Diff details for $FILE";
    IFS=''
    /bin/diff -bBd -y "$HHVM_TMP" "$HH_TMP" | expand | sed -e 's/\\n/\\\\n/g' | while read -r line; do
      OPERATOR="${line:62:1}"
      if [ "$OPERATOR" == "<" ]
      then
        COLOR=${RED}
      elif [ "$OPERATOR" == ">" ]
      then
        COLOR=${GREEN}
      elif [ "$OPERATOR" == "|" ]
      then
        COLOR=${YELLOW}
      else
        COLOR=${NC}
      fi
      echo -e "${COLOR}${line}${NC}"
    done;
    unset IFS
  } >> "$DIFF_TMP"
  rm "$HHVM_TMP"
  rm "$HH_TMP"
done

# log to scuba
$BUCK run //hphp/hack/scripts:log_hh_codegen_results_to_scuba -- "$NUM_LINES_HHVM" "$NUM_LINES_HH" "$NUM_LINES_COMMON" "$1" "$DIFF_TMP"

rm -fr "$DIFF_TMP"
rm -f "/tmp/hhvm_*"
rm -f "/tmp/perf*"
rm -f "/tmp/*.hhas"
