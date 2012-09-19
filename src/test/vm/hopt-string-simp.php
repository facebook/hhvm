<?php
function foo1() {
  $x = "";

  if ($x)
    return 3;
  return 4;
}

function foo2() {
  $x = "5";
  return (int)($x);
}

function foo3() {
  $x = "3";
  return $x == "3";
}

function foo4() {
  $x = 33;
  return $x >= "3";
}

function foo5() {
  $x = "33";
  return $x >= 3;
}

function foo6() {
  $x = "2";
  return 2 != $x;
}

function foo7() {
  $x = "00";
  return $x > "0";
}

function foo8() {
  $x = "";
  return $x == null;
}

function foo9() {
  $x = "-1";
  return $x < null;
}

function foo10() {
  $x = "-1";
  return $x > null;
}

function foo11() {
  $x = "-1";
  return $x < "1";
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
var_dump(foo10());
var_dump(foo11());
