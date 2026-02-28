<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  yield "abc" => "def";
}


<<__EntryPoint>>
function main_g14() :mixed{
$x = foo();
$x->next();
$y = clone $x;
var_dump($x->key() === $y->key());
var_dump($x->current() === $y->current());
}
