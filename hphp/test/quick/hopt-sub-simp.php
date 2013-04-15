<?php

function foo1($x, $y) {
  return $x + (- $y);
}

function foo2($x, $y) {
  return $x - (- $y);
}

function foo3($x) {
  return -$x;
}

function foo4($x) {
  return $x - $x + 3;
}

function foo5($x) {
  return $x + 3 - $x;
}

var_dump(foo1(5, 2));
var_dump(foo2(1, 2));
var_dump(foo3(-3));
var_dump(foo4(5));
var_dump(foo5(5));
