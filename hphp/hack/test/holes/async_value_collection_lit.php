<?hh

async function async_get_an_int(): Awaitable<?int> {
  return 1;
}

function take_a_set(Set<int> $x): void {}

async function async_value_collection_lit(): Awaitable<void> {
  $x = await async_get_an_int();
  /* HH_FIXME[4110] */
  take_a_set(Set {$x});
}
