<?hh // strict

interface Rx {
  <<__Rx>>
  protected function f(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
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
  <<__Rx>>
  protected function f(): int {
    return 2;
  }
}
