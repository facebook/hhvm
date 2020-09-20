<?hh

async function foo(): Awaitable<void> {
  concurrent {
    concurrent {
      $x = await genx();
      await geny();
    }
    await genz();
  }
}
