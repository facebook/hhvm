<?hh // strict

function f(): Generator<string, int, void> {
  yield 'one' => 1;
}
function g(): Generator<string, int, void> {
  yield from f();
  yield from f();
}
