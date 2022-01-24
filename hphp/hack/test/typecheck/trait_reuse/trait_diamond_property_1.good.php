<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Diamond import of traits defining non-generic properties is allowed

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait MyTrait1 {
  public int $myprop = 1;
}

trait MyTrait2 {
  use MyTrait1;
}

<<__EnableMethodTraitDiamond>>
class MyClass {
  use MyTrait1;
  use MyTrait2;
}
