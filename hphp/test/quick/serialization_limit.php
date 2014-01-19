<?php

function foo($a) {
  $v = array();
  for ($i = 0; $i < 1024; $i++) {
    $v[] = $a;
  }
  return $v;
}

function test() {
  $a = foo(1);
  $a = foo($a);
  $a = foo($a);
  print_r($a, true);
}

test();
