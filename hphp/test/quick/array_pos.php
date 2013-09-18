<?php
// test manually created array
function main() {
  $x = null;
  $a = array();
  $a[0] = 'zero';
  $a[1] = 'one';
  var_dump(current($a)); // should print 'zero'
  // test static array
  $a = array('zero', 'one');
  var_dump(current($a));
  // test packed array
  $a = array('zero', ($x ? 'one' : 'one'));
  var_dump(current($a));
}
main();
