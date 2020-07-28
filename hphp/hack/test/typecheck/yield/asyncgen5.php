<?hh // partial

async function foo(): Awaitable<string> {
  return 'hi test';
}

function f(): AsyncGenerator<int, string, void> {
  yield 42 => 'hi test';
}

function expect<T>(T $x):void { }

async function g(): Awaitable<void> {
  foreach (f() await as $x => $y) {
    expect<int>($x);
    expect<string>($y);
  }
}
