<?hh

async function foo(inout int $x) :AsyncGenerator<mixed,mixed,void>{
  yield $x;
}

