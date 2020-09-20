<?hh // strict

interface Rx {
  <<__RxShallow>>
  public function f1(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f1(): int {
    return 1;
  }
}

class B extends A {
  // ERROR, method in interface is more permissive than in base class
  <<__Override>>
  public function f1(): int {
    return 1;
  }
}
