<?php

function test($a, $b, $c, $d, $e) {
  $k = array();
  foreach ($a as $id) {
    $k[$id] = foo($id, $b, $c, $d, $e);
    $k[$id] = foo($k[$id], $b);
  }
}

function foo($a, $b) {
  return $a ?: $b;
}

function main() {
  test(array(array('foo'), array('bar'), array('baz')), null, 1, 2, 3);
}

main();
