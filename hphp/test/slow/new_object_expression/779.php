<?php

class X {
  function __construct($a, &$b) {
  }
}
function test($a) {
  $b = 1;
  return new X($a, $b);
}
var_dump(test(3));
