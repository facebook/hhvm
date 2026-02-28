<?hh
// (c) Meta Platforms, Inc. and affiliates. All Rigths Reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// if __EnableMethodTraitDiamond is selected, then import of traits
// defining generic properties is forbidden

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait MyTrait1<T> {
  public int $myprop = 1;
}
trait MyTrait2 {
  use MyTrait1<int>;
}

<<__EnableMethodTraitDiamond>>
class MyClass {
  use MyTrait1<string>;
  use MyTrait2;
}
