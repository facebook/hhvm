<?hh

async function f1(Awaitable<int> $a): Awaitable<void> {
  $b = (await $a) |> f($$);
}

function f(int $a): int {
  return $a;
}
