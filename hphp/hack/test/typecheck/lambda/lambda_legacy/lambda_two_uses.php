<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo(): int {
    return 3;
  }
}
class D {
  public function foo(): float {
    return 3.2;
  }
}
function test_two_uses(): void {
  $f = $x ==> $x->foo() + 1;
  $a = $f(new C());
  $b = $f(new D());
  $c = $f(new C());
}
