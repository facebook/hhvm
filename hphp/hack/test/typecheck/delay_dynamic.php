<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

async function foo(dynamic $f): Awaitable<void> {
  $_ = delay $f();
}
