<?hh

async function gen_void(): Awaitable<void> {}

async function gen_test(inout string $s): AsyncIterator<int> {
  await gen_void();
  yield 42;
}
