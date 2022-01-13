<?hh

class A {
  public function foo(): void {}
}

trait T1 {
  require extends A;
}

trait T2 {
  public function foo(): void {}
}

trait T3 {
  public function foo(): void {}
}

class B extends A {
  use T1, T2, T3;
}
