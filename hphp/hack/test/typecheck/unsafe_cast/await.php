<?hh

async function foo():Awaitable<int> { return 42; }

async function f(mixed $m): Awaitable<void> {
  $x = await HH\FIXME\UNSAFE_CAST<Awaitable<mixed>, Awaitable<int>>(foo());
}
