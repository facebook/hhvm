<?hh

class A {
  const ctx C = [rx];
}

function f(A $x)[$x::C] :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  throw new Exception();
}

function pure($gen)[]:mixed{
  $gen->next();
  $gen->next();
}

<<__EntryPoint>>
function main() :mixed{
  $gen = f(new A);
  try { pure($gen); } catch (Exception $_) { echo "Caught\n"; }
  $gen->next();
}
