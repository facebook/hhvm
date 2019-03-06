<?hh // strict

async function foo(): Awaitable<int> {
  /* HH_FIXME[4110] */
  return await async { return "string"; };
}
