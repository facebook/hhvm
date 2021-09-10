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
  use T;
}

<<__EnableMethodTraitDiamond>>
trait T3 {
  use T1, T2;
}


class C {
  use T;
}

trait T4 {
  public function foo() : void {
    echo "I am T4\n";
  }
}

<<__EnableMethodTraitDiamond>>
class D extends C {
  use T, T4;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
