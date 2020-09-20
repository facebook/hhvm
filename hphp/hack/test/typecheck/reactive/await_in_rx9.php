<?hh // strict

<<__Rx>>
async function f(int $x): Awaitable<int> {
  return $x;
}

<<__Rx>>
async function g(bool $x): Awaitable<void> {
  // OK
  $a = await ($x ? f(1) : f(2));
}
