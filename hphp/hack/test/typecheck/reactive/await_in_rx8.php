<?hh // strict

<<__Rx>>
async function f(int $x): Awaitable<int> {
  return $x;
}

<<__Rx>>
async function g(): Awaitable<void> {
  // OK
  $a = await (10 |> f($$));
}
