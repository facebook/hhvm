<?hh // strict

function f(): Generator<string, string, void> {
  yield 'one' => 1;
}
