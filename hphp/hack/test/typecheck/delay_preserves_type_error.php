<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 1;
}

function baz(int $x): void {}

async function foo(): Awaitable<void> {
  // preserves Awaitable<int>, passing to int param should fail
  $x = delay bar();
  baz($x);
}
