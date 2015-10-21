<?php

// Test the following:
// - Stepping over yeilds.
// - Stepping over the return from a generator

function bar($a) {
  return $a + 2;
}

function genFoo($a) {
  $a = bar($a);
  $a = bar($a);
  $z = yield $a+5;
  yield $z+1;
  error_log('Finished in genFoo');
}

function foo($a) {
  $gen1 = genFoo($a);
  $gen1->next();
  while ($gen1->valid()) {
    $val = $gen1->current();
    var_dump($val);
    $gen1->send($a);
  }
}

function test($a) {
  foo($a);
}

error_log('flow_gen.php loaded');
