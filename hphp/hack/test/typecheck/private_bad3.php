<?hh // strict

class A {
  private function foo(): int {
    return 4;
  }
}

class B extends A {
  public function bar(): int {
    return parent::foo();
  }
}
