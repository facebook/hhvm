<?hh

async function foreach_async_non_traversable(int $xs): Awaitable<void> {
  /* HH_FIXME[4110] */
  foreach ($xs await as $x) {
  }
}
