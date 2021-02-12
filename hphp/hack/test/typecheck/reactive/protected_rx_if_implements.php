<?hh // strict
interface Rx {

  protected function f(): int;
}

class A {

  protected function f(): int {
    return 0;
  }
}

class B extends A {
  protected function f(): int {
    return 1;
  }
}

class C extends B implements Rx {

  protected function f(): int {
    return 2;
  }
}
