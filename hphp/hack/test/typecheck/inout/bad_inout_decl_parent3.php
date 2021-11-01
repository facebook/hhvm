<?hh

class C1 {
  public function foo(inout int $x): void {}
}

class C2 extends C1 {
  public function foo(int &$x): void {}
}
