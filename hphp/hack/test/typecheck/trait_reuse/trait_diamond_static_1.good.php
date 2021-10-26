<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait MyTrait1 {
  public static function testFun(): void {}
}
trait MyTrait2 {
  use MyTrait1;
}

<<__EnableMethodTraitDiamond>>
class MyClass {
  use MyTrait1;
  use MyTrait2;
}
