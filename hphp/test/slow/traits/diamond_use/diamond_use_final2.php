<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.



trait T {
  public final function foo() : void {
    echo "I am T\n";
  }
}

class C {
  use T;
}

<<__EnableMethodTraitDiamond>>
class D extends C {
  use T;
}

<<__EntryPoint>>
function main() : void {
  (new D())->foo();
}
