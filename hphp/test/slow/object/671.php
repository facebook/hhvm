<?php

class X {
  function foo() {
 var_dump(__METHOD__);
 }
  }
function foo($a) {
  $x = null;
  if ($a) $x = new X;
  return $x;
}
function test($x, $a) {
  $x = foo($a);
  if ($x) {
    $x->foo();
  }
}
test(45, 0);
test(77, 1);
