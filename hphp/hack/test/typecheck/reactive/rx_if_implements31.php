<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(): int {
    return 1;
  }
}

class B extends A {
  <<__Override>>
  public function f(): int {
    // OK since Rx interface defines f as rx
    return 2;
  }
}

class C extends B implements Rx {
  <<__Rx, __Override>>
  public function f(): int {
    // OK - C::f shadows B::f
    return 4;
  }
}
