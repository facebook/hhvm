<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
abstract class B
{
  use TR2;
}

trait TR2 {
  final protected static function foo(
  ): noreturn {
    throw new Exception("A");
  }
  final protected static function bar(
  ): void {
  }
}

class C extends B {
  public function testit(): void {
    static::foo();
    static::bar();
  }
}
