<?hh

function f($x) :AsyncGenerator<mixed,mixed,void>{
  yield $x;
}
trait T {
  function f($x) :AsyncGenerator<mixed,mixed,void>{
 yield static::class;
 }
}
class X {
 use T;
 }

<<__EntryPoint>>
function main_2071() :mixed{
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
