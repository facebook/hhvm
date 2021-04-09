<?hh
async function err(string $x): Awaitable<void> {}
async function f(): Awaitable<int> {
  return 1;
}
async function g(?string $x): Awaitable<int> {
  if ($x === null) {
    return await f();
  }
  /* HH_FIXME[4110] */
  return await err($x);
}
