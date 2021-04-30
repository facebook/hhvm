<?hh

async function foreach_async_union_int_dynamic((int|dynamic) $xs): Awaitable<void> {
  /* HH_FIXME[4110] */
  foreach ($xs await as $x) {
  }
}
