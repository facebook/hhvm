<?hh

async function foo(): Awaitable<string> {
  return 'hi test';
}

function f(): AsyncGenerator<int, string, void> {
  yield 42 => 'hi test';
}

async function g(): Awaitable<void> {
  foreach (f() await as $x => $y) {
    hh_show($x);
    hh_show($y);
  }
}
