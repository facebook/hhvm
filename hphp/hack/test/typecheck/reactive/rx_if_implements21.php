<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function condshallowrx(): int;
}

abstract class A0 extends A {
  // error - if condition is met, shallow is stricter than local
  <<__Override, __RxLocal>>
  public function condshallowrx(): int {
    return 1;
  }
}
