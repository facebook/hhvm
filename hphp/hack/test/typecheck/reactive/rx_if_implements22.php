<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condshallowrx(): int;
}

abstract class A0 extends A {
  // OK
  <<__Override, __RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public function condshallowrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // OK
  <<__Override, __Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function condshallowrx(): int {
    return 1;
  }
}
