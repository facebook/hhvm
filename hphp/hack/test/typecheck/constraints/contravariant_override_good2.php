<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class E<+T> {}

interface I {}
class B {
  public function foo<T as I>(E<T> $x): void {}
}

class C extends B {
  public function foo<T as I>(E<I> $x): void {}
}
