<?hh

class A {
  public function foo(int $_): void {}
}

class B extends A {
  public function foo(arraykey $_): void {}
}

trait T {
  require extends A;
}

class C extends B {
  use T;
}
