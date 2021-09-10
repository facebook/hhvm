<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }
trait T2 { use T; }

// Error, as EnableUnstableFeatures('method_trait_diamond') must be enabled

<<__EnableMethodTraitDiamond>>
class C {
  use T1, T2;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
