<?hh

function f()[rx] {
  yield 1;
  yield 2;
}

function pure($gen)[]{
  echo $gen->next() . "\n";
}

<<__EntryPoint>>
function main() {
  $gen = f();
  $gen->next();
  var_dump($gen->current());
  pure($gen);
}
