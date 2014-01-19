<?php

function foo($a) {
  $x = $a ? 1 : 0;
  return $x - 5;
}
var_dump(foo(1, 2, 3));
var_dump(foo(0, 2, 3));
