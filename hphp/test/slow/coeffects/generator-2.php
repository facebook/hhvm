<?hh

class A {
  const ctx C = [rx];
}

function f(A $x)[$x::C] {
  echo "ok\n";
  yield 1;
  yield 2;
}

function pure($gen)[]{
  echo $gen->next() . "\n";
}

<<__EntryPoint>>
function main() {
  $gen = f(new A);
  $gen->next();
  var_dump($gen->current());
  pure($gen);
}
