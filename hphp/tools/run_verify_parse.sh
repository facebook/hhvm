#!/bin/bash
#
# Run verify checking only parsing works as expected.
#
# Usage:
#
#   % fbconfig hphp/util/parser/test && fbmake dbg && \
#      ./hphp/tools/run_verify_parse.sh
#
HPHP_HOME=$(git rev-parse --show-toplevel)
: ${FBMAKE_BIN_ROOT=_bin}
cd $HPHP_HOME

VERIFY_SCRIPT=./hphp/test/verify
PARSE_TEST=$FBMAKE_BIN_ROOT/hphp/util/parser/test/parse_tester

# some tests are expected not to parse
PARSE_SKIP='dv_i0.php strict_bad_end.php strict_bad_start.php
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

cmd="$PARSE_TEST --verify %1\$s/%3\$s"
exec $VERIFY_SCRIPT --no-exp --command="$cmd" $qtests
