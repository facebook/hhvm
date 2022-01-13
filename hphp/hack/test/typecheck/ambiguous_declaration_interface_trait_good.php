<?hh

class A {
  public function foo(): void {}
}

interface I {
  require extends A;
}

trait T {
  public function foo(): void {}
}

class B extends A implements I {
  use T;
}
