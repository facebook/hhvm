<?hh

function f(): Generator<int, int, void> {
  yield 'one' => 1;
}
