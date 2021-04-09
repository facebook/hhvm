<?hh

async function async_return(): Awaitable<int> {
  /* HH_FIXME[4110] */
  return '3';
}
