<?hh

class A {
  public function f(int &$x): void {}
}

class B extends A {}

class C extends B {
  public function f($x): void {}
}
