<?php

function foo1() {
  return NAN * NAN;
}

function foo2() {
  return NAN * 2;
}

function foo3() {
  return INF * NAN;
}

function foo4() {
  return 2.0 * INF;
}

function foo5() {
  return INF * INF;
}

function foo6($a) {
  return $a * 2.0;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6(3.1));
