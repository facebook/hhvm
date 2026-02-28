<?hh

async function bar(): Awaitable<int> {
  return 42;
}

function baz(string $x): void {}

async function foo(): Awaitable<void> {
  baz(
    /* HH_FIXME[4110] */
    await
    bar()
  );
}
