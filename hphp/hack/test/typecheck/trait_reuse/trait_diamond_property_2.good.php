<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait T {
  public int $myprop = 42;
}

class C {
  use T;
}

<<__EnableMethodTraitDiamond>>
class D extends C {
  use T;
}
