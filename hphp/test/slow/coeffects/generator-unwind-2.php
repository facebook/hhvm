<?hh

class A {
  const ctx C = [rx];
}

async function f(A $x)[$x::C] :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  throw new Exception();
}

async function pure($gen)[]:Awaitable<mixed>{
  $gen->next();
  $gen->next();
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  $gen = f(new A);
  try { await pure($gen); } catch (Exception $_) { echo "Caught\n"; }
  $gen->next();
}
