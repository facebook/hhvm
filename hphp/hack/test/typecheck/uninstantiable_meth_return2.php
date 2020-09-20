<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {}

class C {
  use T;
}

class D {
  public function foo(): T {
    return new C();
  }
}
