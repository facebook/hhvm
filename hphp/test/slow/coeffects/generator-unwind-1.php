<?hh

class A {
  const ctx C = [rx];
}

function f(A $x)[$x::C] {
  yield 1;
  throw new Exception();
}

function pure($gen)[]{
  $gen->next();
  $gen->next();
}

<<__EntryPoint>>
function main() {
  $gen = f(new A);
  try { pure($gen); } catch (Exception $_) { echo "Caught\n"; }
  $gen->next();
}
