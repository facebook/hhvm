<?php

function test($m1, $m2) {
  return array($m1 == $m2);
}

function foo($m1, $m2) {
  $a = test($m1, $m2);
  return $a[0];
}

var_dump(foo(11, 11));
