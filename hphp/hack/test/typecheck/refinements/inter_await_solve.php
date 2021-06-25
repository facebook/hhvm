<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface A {}
class B {
  public function foo(): void {}
}

async function test(
  Awaitable<?A> $a,
): Awaitable<void> {
  $ab = await $a as B;
  $ab = new Inv($ab);
  $ab = $ab->get();
  $ab->foo();
}

class Inv<T> {
  public function __construct(T $_) {}
  public function get(): T {
    throw new Exception();
  }
}
