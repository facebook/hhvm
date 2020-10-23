<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  <<__InferFlows>>
  public static function f(): int {
    return 1;
  }

  <<__InferFlows>>
  public function g(): int {
    return static::f();
  }
}

class B extends A {
  <<__InferFlows>>
  public static function f(): int {
    return 2;
  }
}
