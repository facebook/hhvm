<?hh

async function gen1(): Awaitable<float> { return 42.; }
async function gen2(): Awaitable<void> {}

async function foo(): Awaitable<int> {
  concurrent {
    $x = await gen1();
    await gen2();
  }
  return $x;
}
