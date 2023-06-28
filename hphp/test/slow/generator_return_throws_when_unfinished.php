<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  yield 1; yield 2; yield 3;
  return 11;
}
<<__EntryPoint>> function main(): void {
$g = gen();

$g->next();

var_dump($g->getReturn());
}
