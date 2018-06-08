<?hh // strict

function f(): Generator<int, string, void> {
  yield 'one';
}
function g(): Generator<int, string, void> {
  yield from f();
  yield from f();
}
