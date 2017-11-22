#!/bin/bash

set -e

# In one window, tail your scribe category
#$ ptail -f perfpipe_hackc_hhvm_tests

# In another window, write data to your scribe category
#    (run this command until data shows up in ptail)
#$ scribe_cat perfpipe_hackc_hhvm_tests "{\"int\": {\"time\": $(date +'%s')}}"

TABLE_NAME="perfpipe_hackc_hhvm_tests"
FAIL_COMPARE_QUICK=$(grep -c '^' ~/fbsource/fbcode/hphp/test/hhcodegen_failing_tests_quick)
FAIL_COMPARE_SLOW=$(grep -c '^' ~/fbsource/fbcode/hphp/test/hhcodegen_failing_tests_slow)
FAIL_RUN_QUICK=$(grep -c '^' ~/fbsource/fbcode/hphp/test/hackc_failing_tests_quick)
FAIL_RUN_SLOW=$(grep -c '^' ~/fbsource/fbcode/hphp/test/hackc_failing_tests_slow)

SAMPLE_COMPARE_QUICK="{ \"int\": { \"time\": $(date +'%s'), \"fail\": $FAIL_COMPARE_QUICK }, \"normal\": {\"type\": \"compare_quick\"}}"
SAMPLE_COMPARE_SLOW="{ \"int\": { \"time\": $(date +'%s'), \"fail\": $FAIL_COMPARE_SLOW }, \"normal\": {\"type\": \"compare_slow\"}}"
SAMPLE_RUN_QUICK="{ \"int\": { \"time\": $(date +'%s'), \"fail\": $FAIL_RUN_QUICK }, \"normal\": {\"type\": \"run_quick\"}}"
SAMPLE_RUN_SLOW="{ \"int\": { \"time\": $(date +'%s'), \"fail\": $FAIL_RUN_SLOW }, \"normal\": {\"type\": \"run_slow\"}}"

scribe_cat "$TABLE_NAME" "$SAMPLE_COMPARE_QUICK"
scribe_cat "$TABLE_NAME" "$SAMPLE_COMPARE_SLOW"
scribe_cat "$TABLE_NAME" "$SAMPLE_RUN_QUICK"
scribe_cat "$TABLE_NAME" "$SAMPLE_RUN_SLOW"
