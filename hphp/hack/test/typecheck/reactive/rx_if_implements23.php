<?hh // strict

interface Rx1 {}
interface Rx2 {}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condshallowrx(): int;
}

abstract class A0 extends A {
  // Error, different conditions
  <<__Override, __RxShallow, __OnlyRxIfImpl(Rx2::class)>>
  public function condshallowrx(): int {
    return 1;
  }
}
