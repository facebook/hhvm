<?hh

function f()[rx] :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield 2;
}

function pure($gen)[]:mixed{
  echo (string)($gen->next()) . "\n";
}

<<__EntryPoint>>
function main() :mixed{
  $gen = f();
  $gen->next();
  var_dump($gen->current());
  pure($gen);
}
