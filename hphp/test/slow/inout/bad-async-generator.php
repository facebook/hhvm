<?hh

async function foo(inout int $x) {
  yield $x;
}
