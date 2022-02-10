<?hh

async function await_as_an_expression(): Awaitable<int> {
  $gen1 = async { return 42; };
  $gen2 = async { return 42; };
  $x = (await async { return 42; }) + (await $gen1);
  $y = (await async { return 42; }) + (await async { return 42; });
  return $x;
}
