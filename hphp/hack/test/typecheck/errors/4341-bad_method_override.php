<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function foo(int $x): void {}
}

class B extends A {
  public function foo(string $x): void {}
}
