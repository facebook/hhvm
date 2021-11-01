<?hh

async function f(): AsyncGenerator<int, string, void> {
  yield 'hi';
}
