<?hh // strict

async function foo(): Awaitable<int> {
  // UNSAFE
}

async function bar(): AsyncIterator<int> {
  $x = await foo();
  yield $x;
}

async function test(): Awaitable<void> {
  foreach (bar() await as &$v) {
  }
}
