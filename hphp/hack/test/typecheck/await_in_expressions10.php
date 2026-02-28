<?hh

async function f1(Awaitable<int> $a): Awaitable<void> {
  $b = 10 |> f(await $a, $$);
}
function f(int $a, int $b): int {
  return $a;
}
