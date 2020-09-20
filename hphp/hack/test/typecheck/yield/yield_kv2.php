<?hh // strict

function f(): Generator<int, int, void> {
  yield 'one' => 1;
}
