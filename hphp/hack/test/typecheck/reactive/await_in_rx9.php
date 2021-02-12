<?hh // strict

async function f(int $x)[rx]: Awaitable<int> {
  return $x;
}


async function g(bool $x)[rx]: Awaitable<void> {
  // OK
  $a = await ($x ? f(1) : f(2));
}
