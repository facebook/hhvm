<?php

function foo1() {
  $x = 4;
  $y = 2;
  return $x ^ $y;
}

function foo2() {
  $x = 5;
  $y = false;
  return ($x xor $y);
}

function foo3() {
  $x = 5;
  $y = 5;
  return $x ^ $y;
}

function foo4($x, $y) {
  return $x ^ $y;
}

function foo5($x) {
  return ~$x;
}

function foo6($x) {
  return !$x;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4(3,5));
var_dump(foo5(1));
var_dump(foo6(5));
