<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function bar(): Awaitable<int> {
  return 1;
}

async function baz(): Awaitable<string> {
  return "hello";
}

async function foo(): Awaitable<void> {
  $a = delay bar();
  $b = delay baz();

  $x = await $a;
  $y = await $b;
}
