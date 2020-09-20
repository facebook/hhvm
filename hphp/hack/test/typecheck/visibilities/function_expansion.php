<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  protected function foo(): int {
    return 0;
  }
}

class B extends A {
  public function foo(): int {
    return 1;
  }
}
