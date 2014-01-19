<?php

class X {
  function test($a, $b, $c) {
    return $a != $b;
  }
}
function test($a) {
  $x = new X;
  return $a ? $x->test(1, 2) : false;
}
var_dump(test(1));
