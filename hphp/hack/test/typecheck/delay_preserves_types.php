<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 7;
}

function baz(int $x): void {}

async function foo(): Awaitable<void> {
  // delay preserves Awaitable<int>, then await unwraps it to int
  $x = delay bar();
  $y = await $x;
  baz($y);
}
