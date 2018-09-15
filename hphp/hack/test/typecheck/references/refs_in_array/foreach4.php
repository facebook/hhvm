<?hh // strict

async function foo(): Awaitable<int> {
  // UNSAFE
}

async function bar(): AsyncKeyedIterator<int, int> {
  $x = await foo();
  yield 42 => $x;
}

async function test(): Awaitable<void> {
  foreach (bar() await as $k => &$v) {
  }
}
