<?php

error_reporting(error_reporting() & ~E_NOTICE);

function foo1() {
  $x = true;
  return !($x < false);
}

function foo2($x, $y) {
  return !($x < $y);
}

function foo3() {
  $x = 4;
  return 0 + $x;
}

function foo4() {
  $a = 5;
  $b = 2;
  return $a - $b;
}

function foo5() {
  $x = "ab";
  $y = "a";
  return (int)($x == $y);
}

function foo6($x) {
  return 2 + $x + 2;
}

function foo7($x) {
  $y = $x;
  return $x <= $y;
}

function foo8() {
  $x = array();
  return (bool)$x;
}

function foo9() {
  $x = array(1, 2, 3);
  return (bool)$x;
}

function foo10() {
  $x = array();
  return (string)$x;
}

function foo11() {
  $x = array(1,2,3);
  return (string)$x;
}

var_dump(foo1());
var_dump(foo2(5, 6));
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6(2));
var_dump(foo7(2));
var_dump(foo8());
var_dump(foo9());
var_dump(foo10());
var_dump(foo11());
