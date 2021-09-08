<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  public function foo() : void {
    echo "I am T\n";
  }
}

trait T1 { use T; }

trait T2 {
  use T;
}

trait T3 {
  use T1, T2;
}

trait T4 {
 public function foo() : void {
    echo "I am T4\n";
  }
}

class C {
  use T, T4;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
