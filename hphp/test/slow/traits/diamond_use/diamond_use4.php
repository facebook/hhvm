<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }

trait T2 {
 public function foo() : void {
    echo "I am T2\n";
  }
}

<<__EnableMethodTraitDiamond>>
class C {
  use T, T1, T2;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
