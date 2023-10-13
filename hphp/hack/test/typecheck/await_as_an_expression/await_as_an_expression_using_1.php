<?hh

final class Bar1 implements IDisposable {
  public function __dispose(): void {}
}

final class Bar2 implements IAsyncDisposable {
  public async function __disposeAsync(): Awaitable<void> {}
}

<<__ReturnDisposable>>
async function gen1(): Awaitable<Bar1> {
  return new Bar1();
}

<<__ReturnDisposable>>
async function gen2(): Awaitable<Bar2> {
  return new Bar2();
}

async function foo(): Awaitable<void> {
  using (await gen1()) {
    await async {};
  }
  using ($x = await gen1()) {
    await async {};
  }
  await using (await gen2()) {
    await async {};
  }
  await using ($x = await gen2()) {
    await async {};
  }
}
