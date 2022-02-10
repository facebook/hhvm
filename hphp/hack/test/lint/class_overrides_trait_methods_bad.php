<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait MyTrait2 {
  static public function bar(): int { return 1; }
}

trait MyTrait {
  use MyTrait2;
  public function foo(): int { return 1; }
}

class MyClass {
  use MyTrait;
  <<__Override>>
  public function foo(): int { return 2; }
  <<__Override>>
  static public function bar(): int { return 2; }
}
