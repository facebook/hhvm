<?hh

async function foo(): Awaitable<string> {
  return 'hi test';
}

async function f(): Generator<int, string, void> {
  $x = await foo();
  yield 42 => $x;
}

async function g(): Awaitable<void> {
  foreach (f() await as $x => $y) {
    hh_show($x);
    hh_show($y);
  }
}
