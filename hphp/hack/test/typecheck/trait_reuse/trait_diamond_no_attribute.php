<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }
trait T2 { use T; }

// EnableUnstableFeatures('method_trait_diamond') is no longer required
// to use <<__EnableMethodTraitDiamond>>

<<__EnableMethodTraitDiamond>>
class C {
  use T1, T2;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
