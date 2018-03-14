<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(): int;
}

interface Rx1 extends Rx {
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function f(): int {
    return 1;
  }
}

class B extends A {
  <<__Override>>
  public function f(): int {
    return 2;
  }
}

class C extends B implements Rx1 {
  <<__Rx, __Override>>
  public function f(): int {
    // ERROR, since B::f is not reactive
    return parent::f();
  }
}
