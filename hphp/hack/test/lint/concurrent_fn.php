<?hh

async function concurrent_fn(): Awaitable<int> {
  $gen1 = async { return 42; };
  $gen2 = async { return 42; };
  concurrent {
    $x = await async { return 42; };
    $y = await $gen1;
    await async {};
  }
  return $y;
}
