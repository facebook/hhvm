<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
class A {}

async function test(?A $a): Awaitable<void> {
  $a = await call(
    async () ==> await myGenA()
  );

  if (!$a) {
    return;
  }

  expect<A>($a);
}

function expect<T>(T $_): void {}

async function myGenA(): Awaitable<?A> {
  return null;
}

async function call<T>(
  (function(): Awaitable<T>) $action,
): Awaitable<T> {
  return await $action();
}
