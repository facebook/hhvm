<?hh

function f() :AsyncGenerator<mixed,mixed,void>{
  $a = yield 1;
 list($a, $b) = yield $a;
 yield $b;
}

<<__EntryPoint>>
function main_2156() :mixed{
$c = f();
$c->rewind();
var_dump($c->current());
$c->send(2);
var_dump($c->current());
$c->send(vec[3, 4]);
var_dump($c->current());
}
