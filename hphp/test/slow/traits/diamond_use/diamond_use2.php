<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }
trait T2 { use T; }
trait T3 { use T1; }
trait T4 { use T3; }

class C {
  use T1, T2, T3, T4;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
