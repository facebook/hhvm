<?php

function foo1($a, $b) {
  return ($a - 4) + ($b - 9);
}

function foo2($a, $b) {
  return ($a - 3) + 7;
}

function foo3($a, $b) {
  return ($a - 3) - 7;
}

function foo4($a, $b) {
  return ($a - 4) + ($b + 9);
}

$vals = array(
  array(0, 0),
  array(1, 1),
  array(-3, 5),
  array(5, -3),
  array(20, -50),
  array(20, 0),
);

foreach ($vals as list($a, $b)) {
  var_dump(foo1($a, $b));
  var_dump(foo2($a, $b));
  var_dump(foo3($a, $b));
  var_dump(foo4($a, $b));
}
