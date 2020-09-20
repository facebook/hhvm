<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public static function f(): int {
    return 1;
  }

  public function g(): int {
    return static::f();
  }
}

class B extends A {
  public static function f(): int {
    return 2;
  }
}
