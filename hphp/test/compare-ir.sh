#!/bin/bash
#
# A small script for comparing IR for unit tests from before and after
# a change to the JIT.  Assumes old tracelogs are in *.log.old and new
# ones are in *.log.
#
# Example workflow:
#
#   With previous hhvm:
#   ./test/run -l -m jit ./test/quick
#   find test/ -name \*.log -exec mv {} {}.old \;
#
#   Checkout and build your new hhvm, then:
#   ./test/run -l -m jit ./test/quick
#   ./test/compare-logs.sh
#
# Produces an ir-diffs.diff file that you can inspect for meaningful
# differences in JIT output.

TEST_DIRS=$(dirname $0)

#
# Strip out hex symbols and other extraneous differences
#
find $TEST_DIRS -name \*.log* -exec sed -i \
    -e 's/TCA: 0x[0-9a-f]*/TCA: 0xdeadbeef/g' \
    -e's/TCA: 0x[0-9a-f]*(0x[0-9a-f]*)/TCA: 0xdeadbeef(0xdeadbeef)/g' \
    -e 's/PtrToGen(0x[0-9a-f]*)/PtrToGen(0xdeadbeef)/g' \
    -e 's/Array(0x[0-9a-f]*)/Array(0xdeadbeef)/g' \
    -e 's/NamedEntity(0x[0-9a-f]*)/NamedEntity(0xdeadbeef)/g' \
    -e 's/VerifyParamCls.*/VerifyParamCls/g' \
    -e 's/code-gen.cpp [0-9]* /code-gen.cpp 0000 /g' \
    -e 's/Arr<[0-9]>/Arr/g' {} \;

#
# Diff against the old results
#
find $TEST_DIRS -name \*.log -exec diff -wbBdu {}.old {} \; > ir-diffs.diff
