<?php

function f($x) {

  if (isset(Reference1104::$u)) return null;
  return $x;
}
function test($a) {
  $a++;
  return $a;
}
function foo() {
  return $GLOBALS['x'];
}

<<__EntryPoint>>
function main_1104() {
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
}

abstract final class Reference1104 {
  public static $u;
}
