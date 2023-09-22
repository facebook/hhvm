<?hh

trait T1 {
  abstract public function foo(): void;
}

class A {
  public function foo(): void {}

  public function bar(): void {}
}

abstract class B extends A {
  use T1; // This should be an error, but is not for now

  abstract public function bar(): void;
}
