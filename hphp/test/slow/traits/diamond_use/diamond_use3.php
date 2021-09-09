<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }
trait T3 { use T1; }
trait T4 { use T3; }

trait T2 {
 public function foo() : void {
    echo "I am T2\n";
  }
}

<<__EnableMethodTraitDiamond>>
class C {
  use T1, T2, T3, T4;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
