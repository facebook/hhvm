<?hh

async function f(): AsyncGenerator<string, int, void> {
  yield break;
}
