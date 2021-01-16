<?hh // strict
<<__Rx>>
async function f(int $x)[rx]: Awaitable<int> {
  return $x;
}

<<__Rx>>
async function g()[rx]: Awaitable<void> {
  // OK
  $a = await (10 |> f($$));
}
