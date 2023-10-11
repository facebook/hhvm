<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {
  public function foo() {}
}

class C2 {
  public function foo() {}
}

function test(C1 $c1, C2 $c2, bool $b) {
  if ($b) {
    $x = $c1;
    $x->foo();
  } else {
    $x = $c2;
    $x->foo();
  }
  // this is both C1::foo() and C2::foo(), should hihglight both
  $x->foo();
}
