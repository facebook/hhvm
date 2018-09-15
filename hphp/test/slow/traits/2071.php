<?php

function f($x) {
  yield $x;
}
trait T {
  function f($x) {
 yield get_called_class();
 }
}
class X {
 use T;
 }

<<__EntryPoint>>
function main_2071() {
$c = f(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$x = new X;
$c = $x->f(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$fcn = function ($x) {
 yield $x;
 }
;
$c = $fcn(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
}
