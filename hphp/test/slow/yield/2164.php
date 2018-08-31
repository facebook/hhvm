<?php

function f($x, $y) {
  yield $x;

  yield $$y;
}
function g() {
  extract(&func_get_args(), EXTR_PREFIX_ALL, 'foo');
  yield $foo_0;
  yield $foo_1;
}
function h($x, $y) {
  $z = 16;
  $arr = compact('x', 'y', 'z');
  yield $arr['z'];
  yield $arr['x'];
  yield $arr['y'];
}
function i($x, $y) {
  $arr = compact($x);
  yield $arr[$x];
}

<<__EntryPoint>>
function main_2164() {
foreach (f(10, 'x') as $x) {
 var_dump($x);
 }
foreach (g('hello', 'world') as $x) {
 var_dump($x);
 }
foreach (h(32, 64) as $x) {
 var_dump($x);
 }
foreach (i('y', 32) as $x) {
 var_dump($x);
 }
}
