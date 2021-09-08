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


class C {
  use T;
}

class D extends C {
  use T;
}

<<__EntryPoint>>
function main() : void {
  (new C())->foo();
}
