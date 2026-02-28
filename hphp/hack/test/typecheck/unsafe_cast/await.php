<?hh

async function foo(): Awaitable<int> {
  return 42;
}

async function f(dynamic $m): Awaitable<void> {
  $x = await HH\FIXME\UNSAFE_CAST<dynamic, Awaitable<int>>(foo());
}
