<?hh // strict
class A {
  public function foo(): void {}
}

class B extends A {
  // should fail
  public function FOO(): void {}
}
