<?php


function f($x) {
  yield $x;
}
$c = f(32);
var_dump($c->getOrigFuncName());
trait T {
  function f($x) { yield get_called_class(); }
}
class X { use T; }
$x = new X;
$c = $x->f(32);
var_dump($c->getOrigFuncName());
$fcn = function ($x) { yield $x; };
$c = $fcn(32);
var_dump($c->getOrigFuncName());
