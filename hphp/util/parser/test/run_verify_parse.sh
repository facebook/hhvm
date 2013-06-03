#!/bin/bash
#
# Run verify checking only parsing works as expected.
#
# Usage:
#
#   % fbconfig hphp/util/parser/test && fbmake dbg && \
#      ./hphp/util/parser/test/run_verify_parse.sh
#
HPHP_HOME=$(git rev-parse --show-toplevel)
: ${FBMAKE_BIN_ROOT=_bin}
cd $HPHP_HOME

VERIFY_SCRIPT=./hphp/test/run
HHVM_BIN=$FBMAKE_BIN_ROOT/hphp/util/parser/test/parse_tester

# some tests are expected not to parse
PARSE_SKIP='dv_i0.php hh_bad_end.php hh_bad_start.php hh_numbers.php
  strict_bad_end.php strict_bad_start.php
  strict_numbers.php syntax-error.php xhp-malformed.php Xhp.php
  trailing_comma_bad1.php trailing_comma_bad2.php trailing_comma_bad3.php
  trailing_comma_bad4.php trailing_comma_bad5.php trailing_comma_bad6.php'
PARSE_SKIP="$PARSE_SKIP $(cd hphp/test/quick && ls parse_fail_*.php)"

######################################################################


skip_list=
for x in $PARSE_SKIP ; do
    skip_list="$skip_list hphp/test/quick/$x"
done

qtests=$(comm -23 \
    <(find hphp/test/quick -maxdepth 1 -name \*.php | sort) \
    <(echo $skip_list|sed -e 's/ /\n/g'|sort))

HHVM_BIN=$HHVM_BIN $VERIFY_SCRIPT $qtests
