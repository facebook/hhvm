<?php

function f($x, $y) {
  $z = 32;
  return $x && $y ?: $z;
}
var_dump(f(false, false));
var_dump(f(true, true));
