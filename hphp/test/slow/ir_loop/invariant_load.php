<?php

function alternator() {
  static $i = 0;
  mt_rand();
  mt_rand();
  mt_rand();
  return ($i++ % 2) == 0;
}
function foo($x, $k) {
  foreach ($x as $j) {
  loop:
    echo "$j: ";
    echo $k;
    echo "\n";
    if (alternator()) { goto loop; }
    echo "ok\n";
  }

  echo "done\n";
}

foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 'asd');
