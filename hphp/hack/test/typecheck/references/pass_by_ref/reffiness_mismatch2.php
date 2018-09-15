<?hh

class A {
  public function f(int &$x): void {}
}

class B extends A {
  public function f(int $x): void {}
}
