<?php

function f(&$x) { var_dump($x); }
function test($b, $c) {
  $x = false && $b;
  $x += true && $b;
  $x += false || $b;
  $x += true || $b;

  $x += false ? $b : $c;
  $x += true ? $b : $c;
  f($x);
}

test(2, 3);
