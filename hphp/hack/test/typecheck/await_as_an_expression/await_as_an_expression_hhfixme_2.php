<?hh

async function bar(): Awaitable<int> {
  return 42;
}

async function foo(): Awaitable<int> {
  return await
    /* HH_FIXME[4105] */
    bar('oops');
}
