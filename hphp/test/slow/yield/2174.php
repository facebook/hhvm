<?php


function f($x) {
  yield $x;
}
$c = f(32);
var_dump($c->getOrigFuncName());
class X {
  function f($x) { yield $x; }
  static function g($x) { yield get_called_class(); }
}
class Y extends X {}
$x = new X;
$c = $x->f(32);
var_dump($c->getOrigFuncName());
$c = X::g(32);
var_dump($c->getOrigFuncName());
$c = Y::g(32);
var_dump($c->getOrigFuncName());
$fcn = function ($x) { yield $x; };
$c = $fcn(32);
var_dump($c->getOrigFuncName());
