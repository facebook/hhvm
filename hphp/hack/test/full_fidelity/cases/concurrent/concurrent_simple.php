<?hh

async function foo(): Awaitable<void> {
  concurrent {
    $x = await genx();
    await geny();
  }
}
