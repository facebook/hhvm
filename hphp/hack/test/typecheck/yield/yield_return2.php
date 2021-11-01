<?hh

function f(): Generator<int, int, void> {
  while (true) {
    yield 1;
    return 'hi';
  }
}
