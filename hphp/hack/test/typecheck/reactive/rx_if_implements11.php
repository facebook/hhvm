<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: conflicting annotations
  <<__RxIfImplements(Rx1::class), __RxLocalIfImplements(Rx1::class)>>
  public abstract function mayberx2(): void;
}
