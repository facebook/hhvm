#!/bin/bash
#
# Run the verify_quick test suite in various configurations.
#
# Main interface is controlled by the VQ env var:
#
#    VQ=jit    ./run_verify.sh
#    VQ=hhir   ./run_verify.sh
#    VQ=interp ./run_verify.sh
#    ./run_verify.sh  # default runs the jit
#
# Server mode flags:  (semi-functional--tests with warnings fail)
#
#    MODE="server" -- run in server mode
#    PORT          -- port to use in server mode (default 8080)
#    TEST_HOME     -- the --home arg for server mode
#
# Other flags:
#
#    VERIFY_EXTRA_ARGS      -- extra args to pass to verify
#    VERIFY_EXTRA_CMD_ARGS  -- extra args to pass to hhvm
#
# For use in the Makefile
#
#    HHVM          -- path to the hhvm binary to use
#    VERIFY_HHBC   -- path to use for the an hhbc repo
#

######################################################################

: ${FBMAKE_BIN_ROOT=$HPHP_HOME/_bin}

DEFAULT_VERIFY_HHBC=$FBMAKE_BIN_ROOT/verify.hhbc
DEFAULT_HHVM=$FBMAKE_BIN_ROOT/hphp/hhvm/hhvm

QTESTS_SKIP='autoload5.php condinfinite.php condinfinite2.php define_b.php
  _fbcufa_init.php include_b.php include_c.php include_d.php
  include2_a.php include2_b.php infinite.php ReqOnce_b.php include3_a.php
  include_backtrace2.php backup_cycle_collector.php
  redeclared_class1.php redeclared_class2.php'

VERIFY_SCRIPT=./test/verify

######################################################################

if [ x"$HPHP_HOME" = x"" ] ; then
    echo "HPHP_HOME must be set in your environment"
    exit 1
fi

: ${HHVM:=$DEFAULT_HHVM}
: ${VERIFY_HHBC:=$DEFAULT_VERIFY_HHBC}
: ${VQ:=jit}
: ${TEST_PATH:=test/vm}

cd $HPHP_HOME/hphp

######################################################################

skip_list=
for x in $QTESTS_SKIP ; do
    skip_list="$skip_list $TEST_PATH/$x"
done

qtests=$(comm -23 \
    <(find $TEST_PATH -name \*.php -o -name \*.hhas | sort) \
    <(echo $skip_list|sed -e 's/ /\n/g'|sort))

repo_args="-v Repo.Local.Mode=-- -v Repo.Central.Path=$VERIFY_HHBC"
interp_args="$repo_args -v Eval.Jit=false"
jit_args="$repo_args -v Eval.Jit=true -v Eval.JitEnableRenameFunction=true"
hhir_args="$jit_args -v Eval.JitUseIR=true -v Eval.HHIRDisableTx64=true"

######################################################################

cmd="$HHVM --config test/cli.hdf "
if [ x"$MODE" = x"server" ] ; then
    : ${PORT:=8080}
    : ${TEST_HOME:=.}
    cmd="$cmd -m server -v Server.SourceRoot=%s -p %s"
    verify_args="--server --port $PORT --home $TEST_HOME"
else
    cmd="$cmd --file %3\$s"
fi
verify_args="$verify_args --hhvm $HHVM"

cmd="$cmd -v Eval.EnableObjDestructCall=true"

echo $TEST_PATH | grep -q zend
if [ $? -ne 0 ] ; then
    cmd="$cmd -v Eval.AllowHhas=true \
              -v Eval.EnableHipHopSyntax=true"
fi

case $VQ in
    jit)
        cmd="$cmd $jit_args"
        ;;
    hhir)
        cmd="$cmd $hhir_args"
        ;;
    interp)
        cmd="$cmd $interp_args"
        ;;
    *)
        cmd="$cmd $jit_args"
        ;;
esac

exec $VERIFY_SCRIPT --command="$cmd $VERIFY_EXTRA_CMD_ARGS" $verify_args \
    $VERIFY_EXTRA_ARGS $qtests
