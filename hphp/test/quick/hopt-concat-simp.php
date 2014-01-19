<?php
function foo1() {
  $x = "ab";
  $y = "c";
  return $x . $y;
}

function foo2() {
  $x = "x";
  $y = "y";
  $z = "z";

  return $x . $y . $z;
}

function foo3($x) {
  $y = "c";
  return $x . $y;
}

var_dump(foo1());
var_dump(foo2());
var_dump(foo3("ab"));
