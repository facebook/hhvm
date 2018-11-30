<?hh // strict

function f<T>(T $x): Generator<string, T, void> {
  yield 'one' => $x;
}
function g(): Generator<string, int, void> {
  yield from f(23);
  yield from f("a");
}
