<?hh // strict

function f(): Generator<string, int, void> {
  yield 'one' => 1;
}
