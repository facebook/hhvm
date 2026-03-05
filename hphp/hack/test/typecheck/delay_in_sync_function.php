<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 42;
}

function foo(): void {
  delay bar();
}
