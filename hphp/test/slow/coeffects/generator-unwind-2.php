<?hh

class A {
  const ctx C = [rx];
}

async function f(A $x)[$x::C] {
  yield 1;
  throw new Exception();
}

async function pure($gen)[]{
  $gen->next();
  $gen->next();
}

<<__EntryPoint>>
async function main() {
  $gen = f(new A);
  try { await pure($gen); } catch (Exception $_) { echo "Caught\n"; }
  $gen->next();
}
