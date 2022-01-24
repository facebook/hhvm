<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Diamond import of traits defining generic properties is forbidden

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait MyTrait1<T> {
  public ?T $myprop = null;
}
trait MyTrait2 {
  use MyTrait1<int>;
}

<<__EnableMethodTraitDiamond>>
class MyClass {
  use MyTrait1<string>;
  use MyTrait2;
}
