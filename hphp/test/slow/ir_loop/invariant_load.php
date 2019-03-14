<?php

abstract final class AlternatorStatics {
  public static $i = 0;
}

function alternator() {
  mt_rand();
  mt_rand();
  mt_rand();
  return (AlternatorStatics::$i++ % 2) == 0;
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


<<__EntryPoint>>
function main_invariant_load() {
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 123);
foo(array(1,2,3), 'asd');
}
