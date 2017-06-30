#!/bin/bash

# This script semdiff files with similar names e.g. myfile.1.hhas and
# myfile.2.hhas, and compares the result with the corresponding expected
# file e.g. myfile.semdiff.exp, or creates it if it does not exists with
# a .out extensnion e.g. myfile.semdiff.exp.out

if [ "$1" ]; then
  echo 'ERROR: unexpected argument'
  echo 'usage: run_semdiff_tests.sh'
  exit 1
fi

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

BASEDIR="$HOME/fbsource/fbcode"
BUCK="$BASEDIR/tools/build/buck/bin/buck"
DIFF_TMP=/tmp/semdiff_tmp.exp
DIFF_RESULT=/tmp/diff_tmp.result

# prepare semdiff
echo "compiling semdiff"
"$BUCK" build @mode/dbgo //hphp/hack/src/hhbc/semdiff:semdiff 2> /dev/null
SEMDIFF_PATH=$("$BUCK" targets --show-output //hphp/hack/src/hhbc/semdiff:semdiff 2> /dev/null | cut -d' ' -f2)
SEMDIFF_PATH="$BASEDIR/$SEMDIFF_PATH"
echo "retrieving path: $SEMDIFF_PATH"

HHAS_FILES=$(find "$BASEDIR"/hphp/hack/test/semdiff -type f -name '*.1.hhas')

NUM_SUCCESS=0
NUM_FAIL=0
for FILE in $HHAS_FILES; do
  basename=${FILE%.1.hhas}
  test_name=${basename##*/}
  "$SEMDIFF_PATH" "$FILE" "$basename.2.hhas" > "$DIFF_TMP"
  if [ ! -f "$basename.semdiff.exp" ]
  then
    new_file="$basename.semdiff.exp.out"
    echo -e "\tcreating $YELLOW $test_name.semdiff.exp.out $NC"
    mv "$DIFF_TMP" "$new_file"
  else
    test_name=${basename##*/}
    printf "\tdiff'ing %s:\t" "$test_name"
    diff "$DIFF_TMP" "$basename.semdiff.exp" > "$DIFF_RESULT"
    if [ -z "$(cat $DIFF_RESULT)" ]
    then
      echo -e "$GREEN OK $NC"
      NUM_SUCCESS=$((NUM_SUCCESS + 1))
    else
      echo -e "$RED KO $NC"
      NUM_FAIL=$((NUM_FAIL + 1))
    fi
  fi
done

echo -e "successes:\t$GREEN $NUM_SUCCESS $NC / $((NUM_SUCCESS + NUM_FAIL))"
echo -e "failures:\t$RED $NUM_FAIL $NC / $((NUM_SUCCESS + NUM_FAIL))"
