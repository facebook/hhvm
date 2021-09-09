<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('method_trait_diamond')>>

class C {
  const int X = 1;
}

trait T {
  require extends C;
  public function foo() : int { return $this::X; }
}

trait T1 {
  use T;
  const int X = 1;
}

trait T2 {
  use T;
  const int X = 2;
}

<<__EnableMethodTraitDiamond>>
class D extends C {
  use T1, T2;
}

<<__EntryPoint>>
function main(): void {
  echo (new D())->foo() . "\n";
}
