<?php

function f($x) {
  global $u;
  if (isset($u)) return null;
  return $x;
}
function test($a) {
  $a++;
  return $a;
}
function &foo() {
  return $GLOBALS['x'];
}
$x = 1;
test(foo());
var_dump($x);
$f = f('foo');
$x = 1;
test($f());
var_dump($x);
$t = f('test');
$x = 1;
$t(foo());
var_dump($x);
