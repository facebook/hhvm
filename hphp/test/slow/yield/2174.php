<?hh

function f($x) {
  yield $x;
}
class X {
  function f($x) {
    yield $x;
  }
  static function g($x) {
    yield static::class;
  }
}
class Y extends X {
}

<<__EntryPoint>>
function main_2174() {
$c = f(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$x = new X;
$c = $x->f(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$c = X::g(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$c = Y::g(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
$fcn = function ($x) {
  yield $x;
};
$c = $fcn(32);
var_dump($c->getOrigFuncName());
var_dump($c->getCalledClass());
}
