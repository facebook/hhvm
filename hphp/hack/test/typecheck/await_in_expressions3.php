<?hh // strict

async function f1(Awaitable<int> $a): Awaitable<void> {
  f2(await $a);
}
function f2(int $x): int {
  return $x;
}
