<?hh

async function f(): AsyncGenerator<string, string, void> {
  yield 1 => 'two';
}
