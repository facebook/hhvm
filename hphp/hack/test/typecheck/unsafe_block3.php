<?hh

function f(): Generator<int, int, void> {
  {
    // UNSAFE
  }
  yield 1;
}
