<?php

function f(&$u0) {
  return function () use (&$u0, $u0) {
 $u0++;
 }
;
}
function g(&$u0) {
  return function () use ($u0, &$u0) {
 $u0++;
 }
;
}
$x1 = 0;
$f = f($x1);
var_dump($x1);
$f();
var_dump($x1);

$x2 = 0;
$g = g($x2);
var_dump($x2);
$g();
var_dump($x2);
