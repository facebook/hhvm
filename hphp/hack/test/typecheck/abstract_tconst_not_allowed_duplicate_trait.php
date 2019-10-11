<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T;
}

trait MyTrait {
  public function foo(A::T $_): void {}
}

class B {
  use MyTrait;
}
