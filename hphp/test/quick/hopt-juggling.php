<?php

function foo1() {
  $x = true;
  return $x + true;
}

function foo2() {
  $x = true;
  return $x + null;
}

function foo3() {
  $x = "5";
  return $x + 3;
}

function foo4() {
  $x = true;
  return $x - true;
}

function foo5() {
  $x = true;
  return $x * true;
}

function foo6() {
  $x = "13";
  return $x ^ 6;
}

function foo7() {
  $x = "7";
  return $x & false;
}

function foo8() {
  $x = "4";
  return $x * "6";
}

function foo9() {
  $x = "3";
  return $x - null;
}

function foo10($x) {
  return $x + "10";
}

function foo11($x, $y) {
  return $x + $y;
}

function foo12() {
  $x = array();
  return (int)$x;
}

function foo13() {
  $x = array(1,2,3);
  return (int)$x;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
var_dump(foo8());
var_dump(foo9());
var_dump(foo10(6));
var_dump(foo11(4, true));
var_dump(foo12());
var_dump(foo13());
