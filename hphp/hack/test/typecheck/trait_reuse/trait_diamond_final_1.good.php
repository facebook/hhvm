<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait T {
  public final function f(): void {}
}

trait T1 { use T; }
trait T2 { use T; }

<<__EnableMethodTraitDiamond>>
class C {
  use T1, T2;
}
