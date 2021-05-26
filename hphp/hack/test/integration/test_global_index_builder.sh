#!/bin/bash

CMD="$1"
REPO="$2"

NAME=autocomplete
TEMP_REPO="$(mktemp -d)"
TEMP_FILE="${TEMP_REPO}/${NAME}.txt"
touch "${TEMP_FILE}"
export HH_TEST_MODE=1  # avoid writing a bunch of telemetry
echo "CHECKING ${TEMP_REPO} ${TEMP_FILE}"

# Build a global index in text
if ! "${CMD}" --text "${TEMP_FILE}" "${REPO}"; then
  echo "FAILURE: ${CMD} failed to run"
  exit 1
fi

# Ensure that we have at least 5,300 lines - new HHIs may be added over time so
# don't throw a failure if the line count increases
LINE_COUNT=$(wc "${TEMP_FILE}" | cut -d " " --fields=3)
if [ "${LINE_COUNT}" -gt 5300 ]; then
  echo "Line count is good"
else
  echo "FAILURE: Line count should be greater than 5300, since"
  echo "         HHI builtins are included in this task."
  echo "         If the file has shrunk in size, the global"
  echo "         index builder is probably not catching HHIs."
  exit 1
fi


function spot_check() {
  expected=$1
  matches=0
  matches=$(grep "${expected}" "${TEMP_REPO}/autocomplete.txt" -c)
  if [ "${matches}" -ne 1 ]; then
    echo "FAILURE: Expected to find the symbol:"
    echo "         [${expected}]"
    echo "         Please check to see if this symbol still exists in"
    echo "         //hphp/hack/test/integration/data/simple_repo"
    echo "         If it does not, please comment out this test.  If"
    echo "         the symbol still exists, then hh_global_index_builder"
    echo "         may be failing."
    exit 1
  else
    echo "Searching for [${expected}] found [${matches}] matches"
  fi
}


# Run some spot tests to ensure certain things made it into the file
spot_check "Aaaaaaaaaaa_class 1 acid  acnew  actype"
spot_check "Derp\\\\Lib\\\\Herp\\\\f"
spot_check "__CLASS__ 9 acid"
spot_check ":xhp:helloworld 1 acid"
spot_check "SymbolInsideHackFile 1 acid  acnew  actype"

# Clean up
rm -rf "${TEMP_REPO}"
