<?hh

async function f(): AsyncGenerator<?int, int, void> {
  while (true) {
    yield 1;
    return 'hi';
  }
}
