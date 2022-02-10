<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait MyTrait {
  abstract public function foo(): int;
  abstract static public function bar(): int;
}

class MyClass {
  use MyTrait;
  <<__Override>>
  public function foo(): int { return 2; }
  <<__Override>>
  static public function bar(): int { return 2; }
}
