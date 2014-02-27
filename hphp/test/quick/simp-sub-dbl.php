<?php

function foo1() {
  return NAN - NAN;
}

function foo2() {
  return 5.5 - NAN;
}

function foo3() {
  return NAN - 5.5;
}

function foo4($a) {
  return $a - 4.5;
}

function foo5() {
  return INF - 6.1;
}

function foo6() {
  return 4.4 - INF;
}

function foo7() {
  return INF - INF;
}

function foo8() {
  return NAN - INF;
}

function foo9() {
  return INF - NAN;
}

function foo10(double $a) {
  return $a - $a;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3());
var_dump(foo4(3.1));
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
var_dump(foo8());
var_dump(foo9());
var_dump(foo10(NAN));
