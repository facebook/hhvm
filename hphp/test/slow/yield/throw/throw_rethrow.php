<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{
  echo "before yield\n";
  try {
    yield;
  } catch (RuntimeException $e) {
    echo 'Caught: ', $e, "\n\n";

    throw new LogicException('new throw');
  }
}
<<__EntryPoint>> function main(): void {
$gen = gen();
$gen->next();
var_dump($gen->throw(new RuntimeException('throw')));
}
