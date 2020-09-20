<?hh
async function foo1(): Awaitable<void> {
  await g();
  concurrent {
    $x = await genx();
    await geny();
  }
  await h();
}
