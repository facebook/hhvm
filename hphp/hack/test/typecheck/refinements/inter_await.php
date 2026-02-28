<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function foo(): void {}
}

async function test(Awaitable<?A> $x): Awaitable<void> {
  $i = (await $x) as nonnull;
  expect<A>($i);
}

function expect<T>(T $_): void {}
