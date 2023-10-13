<?hh


async function genx(): Awaitable<int> { return 42; }

async function geny(): Awaitable<void> {}


async function foo(): Awaitable<int> {
  concurrent {
    $x = await genx();
    await geny();
    $y = await genx();
  }
  return $x + $y;
}
