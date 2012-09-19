<?php

function foo1() {
  $x = 2;
  return $x === "2";
}

function foo2() {
  $x = 2;
  return $x !== "2";
}

function foo3() {
  $x = "2";
  return $x === 2;
}

function foo4() {
  $x = "2";
  return $x !== 2;
}

function foo5() {
  $x = 2;
  return $x == "2";
}

function foo6() {
  $x = 2;
  return $x != "2";
}

function foo7() {
  $x = "2";
  return $x == 2;
}

function foo8() {
  $x = "2";
  return $x != 2;
}

function foo9($x) {
  $y = $x + 1;
  return $y === 6;
}

function foo10($x) {
  $y = $x + 1;
  return $y !== 6;
}

function foo11($x) {
  $y = $x xor 0;
  return $y === 6;
}

function foo12($x) {
  return $x !== 6;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
var_dump(foo8());
var_dump(foo9(5));
var_dump(foo10(5));
var_dump(foo11("5"));
var_dump(foo12("6"));
