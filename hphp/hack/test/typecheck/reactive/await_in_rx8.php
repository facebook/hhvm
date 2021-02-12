<?hh // strict

async function f(int $x)[rx]: Awaitable<int> {
  return $x;
}


async function g()[rx]: Awaitable<void> {
  // OK
  $a = await (10 |> f($$));
}
