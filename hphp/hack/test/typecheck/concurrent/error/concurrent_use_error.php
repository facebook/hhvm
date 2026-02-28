<?hh

async function foo(): Awaitable<void> {
  concurrent {
    $x = await async {};
    await async {};
  }
}
