<?hh // strict

final class Bar implements IAsyncDisposable {
  public async function __disposeAsync(): Awaitable<void> {}
}

<<__ReturnDisposable>>
async function gen1(): Awaitable<Bar> {
  return new Bar();
}

<<__ReturnDisposable>>
async function gen2(): Awaitable<Bar> {
  return await gen1();
}

async function foo(): Awaitable<void> {
  await using (await gen2()) {
    await async {};
  }
  await using ($x = await gen2()) {
    await async {};
  }
}
