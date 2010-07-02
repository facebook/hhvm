#!/bin/sh
#
test_files=`ls $HPHP_HOME/src/test/tainting/*.php`
for i in $test_files
do
 echo "Testing file $i:"
 $HPHP_HOME/src/hphpi/hphpi -f $i | pretty | diff $i.exp -
done
echo 'If you have seen some diffs, then something is wrong'
