<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: conflicting annotations
  <<__Rx, __RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx2(): void;
}
