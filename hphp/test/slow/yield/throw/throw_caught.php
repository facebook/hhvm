<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  echo "before yield\n";
  try {
    yield;
  } catch (RuntimeException $e) {
    echo $e->__toString(), "\n\n";
  }

  yield 'result';
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
$gen->throw(new RuntimeException('Test'));
var_dump($gen->current());
}
