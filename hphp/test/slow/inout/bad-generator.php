<?hh

function foo(inout int $x) {
  yield $x;
}

<<__EntryPoint>> function main(): void {}
