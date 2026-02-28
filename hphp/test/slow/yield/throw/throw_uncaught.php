<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 'thisThrows';
  yield 'notReached';
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
var_dump($gen->throw(new RuntimeException('test')));
}
