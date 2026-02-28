<?hh

function foo() :AsyncGenerator<mixed,mixed,void>{
  yield 1 => 2;
  yield "a" => "b";
}
<<__EntryPoint>> function main(): void {
$gen = foo();
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
$gen->next();
var_dump($gen->key());
var_dump($gen->current());
}
