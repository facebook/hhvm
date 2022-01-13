<?hh

class A {
  public function foo(): void {}
}

trait T1 {
  public function foo(): void {}
}

trait T2 {
  public function foo(): void {}
}

trait T3 {
  require extends A;
}

class B extends A {
  use T1, T2, T3;
}
