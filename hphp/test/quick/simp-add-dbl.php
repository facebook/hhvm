<?php

function foo1(bool $a) {
  return $a + 1.1;
}

function foo2() {
  return NAN + NAN;
}

function foo3() {
  return NAN + 0.0;
}

function foo4() {
  return NAN + 1.1;
}

function foo5() {
  return NAN + INF;
}

function foo6() {
  return INF + INF;
}

function foo7() {
  return 6 + INF;
}

var_dump(foo1(true));
var_dump(foo2());
var_dump(foo3());
var_dump(foo4());
var_dump(foo5());
var_dump(foo6());
var_dump(foo7());
