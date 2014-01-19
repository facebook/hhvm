<?php

function test() {
  $a = array();
  for ($i = 0;
 $i < 17;
 $i++) {
    $a[] = $i;
  }
  unset($a[16]);
  $b = $a;
  array_unshift($a, 'foo');
  var_dump(count($a), count($b));
}
test();
