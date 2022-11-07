<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// __EnableMethodTraitDiamond is no longer an experimental feature, so this is supported.

trait MyTrait1 {
  public function testFun(): void {}
}
trait MyTrait2 {
  use MyTrait1;
}

<<__EnableMethodTraitDiamond>>
class MyClass {
  use MyTrait1;
  use MyTrait2;
}
