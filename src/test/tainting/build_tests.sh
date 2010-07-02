#!/bin/sh
#
# This script runs tainting tests on all the taint<i>.php and stores the
# ouptut in taint<i>.php.exp. Please do not run it if you are not sure the
# tainting features work, as it resets the expected outputs of the test cases.
#
# The script is not executable so that it is not run by accident. To run it do:
# sh built_tests.sh
#
read -n 1 -p 'This test will recreate the .exp (for expected) files. Are you sure you want to proceed? [y/n]' answer
echo ''
if [ $answer = 'y' ]
then
  test_files=`ls $HPHP_HOME/src/test/tainting/*.php`
  for i in $test_files
  do
    echo "--- Building $i.exp ---"
    $HPHP_HOME/src/hphpi/hphpi -f $i | pretty > $i.exp
  done
fi
