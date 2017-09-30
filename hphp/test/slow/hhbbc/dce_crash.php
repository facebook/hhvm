<?php

function f($x) { return $x; }

function g($t) {
  $a = 1;
  $b = 1;
  return f($t ? $a : $b);
}

var_dump(g(true));
