<?php

$x = 32;
$SOME_VAR = 'foo';
function f($a0, $a1, $a2, $a3) {
  var_dump($a0['SOME_VAR'], $a1, $a2, $a3);
}
function g($a0, $a1, $a2, $a3) {
  var_dump($a0['SOME_VAR'], $a1, $a2, $a3);
}
function h($fcn) {
  global $x;
  $fcn($GLOBALS, $_POST, $x, $x++);
  yield 64;
}
foreach (h(rand(0, 1) ? 'f' : 'g') as $v) {
  var_dump($v);
}
