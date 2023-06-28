<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

final class A {
  public vec $p1;
  public function __construct() {
    $this->p1 = __hhvm_intrinsics\launder_value(vec[1, 2, 3]);
  }
}

final class B {
  private A $p2;
  public function __construct() {
    $this->p2 = new A();
  }
  public function foo() :mixed{
    return $this->p2->p1 |> dict($$);
  }
}

<<__EntryPoint>> function test(): void {
  $b = new B();
  var_dump($b->foo());
}
