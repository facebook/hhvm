<?hh

class A {
  const ctx C = [rx];
}

function f<reify T>(A $x)[$x::C] :AsyncGenerator<mixed,mixed,void>{
  echo "ok\n";
  yield 1;
  yield 2;
}

function pure($gen)[]:mixed{
  echo (string)($gen->next()) . "\n";
}

<<__EntryPoint>>
function main() :mixed{
  $gen = f<int>(new A);
  $gen->next();
  var_dump($gen->current());
  pure($gen);
}
