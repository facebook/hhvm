<?hh // strict
interface Rx {}

class A {

  public function f(): int {
    return 1;
  }
}

class B extends A implements Rx {
}

class C extends B {

  public function g(): int {
    // should be OK
    return parent::f();
  }
}
