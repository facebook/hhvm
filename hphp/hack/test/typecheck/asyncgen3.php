<?hh // partial

async function foo(): Awaitable<string> {
  return 'hi test';
}

function f(): AsyncGenerator<int, string, void> {
  $x = await foo();
  yield 42 => $x;
}

function expect<T>(T $x): void { }

async function g(): Awaitable<void> {
  foreach (f() await as $x => $y) {
    expect<int>($x);
    expect<string>($y);
  }
}
