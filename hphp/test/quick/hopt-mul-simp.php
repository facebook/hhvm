<?php
function foo1() {
  $x = 3;
  $y = 4;
  return $x * $y;
}

function foo2($x) {
  return $x * 2;
}

function foo3($x, $y) {
  return $x * $y;
}

function foo4($x) {
  return (-1) * $x;
}

function foo5($x, $y, $z) {
  return $x * $y + $x * $z;
}

function foo6($x, $y, $z) {
  return $y * $x + $x * $z;
}

function foo7($x, $y, $z) {
  return $x * $y + $z * $x;
}

function foo8($x, $y, $z) {
  return $y * $x + $z * $x;
}

function foo9($x, $y) {
  return ($x * 3) * ($y * 7);
}

function foo10($x) {
  return (3 * $x) * 7;
}

var_dump(foo1());
var_dump(foo2(6));
var_dump(foo3(6,2));
var_dump(foo4(-12));
var_dump(foo5(2,1,5));
var_dump(foo6(2,1,5));
var_dump(foo7(2,1,5));
var_dump(foo8(2,1,5));
var_dump(foo9(2,5));
var_dump(foo10(2));
