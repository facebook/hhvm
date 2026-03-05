<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 42;
}

async function foo(): Awaitable<void> {
  $_ = delay bar();
}
