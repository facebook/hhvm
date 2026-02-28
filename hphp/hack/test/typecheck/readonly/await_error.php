<?hh

async function f(int $i): Awaitable<void> {}

async function g(readonly int $i): Awaitable<void> {
  concurrent {
    await f($i);
    await f(1);
  }
}
