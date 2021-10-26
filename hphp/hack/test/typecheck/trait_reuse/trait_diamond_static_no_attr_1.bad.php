<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// ERROR: variant of T81286015 with static methods (HHVM fatals also on static methods)

trait MyTrait1 {
  public static function testFun(): void {}
}
trait MyTrait2 {
  use MyTrait1;
}

class MyClass {
  use MyTrait1;
  use MyTrait2;
}
