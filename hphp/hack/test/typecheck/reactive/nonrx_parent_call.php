<?hh // strict

interface Rx {}

class A {
  public function f(): int {
    return 1;
  }
}

class B extends A {
  <<__Rx, __Override>>
  public function f(): int {
    // should be error, A is not reactive
    return parent::f();
  }
}
