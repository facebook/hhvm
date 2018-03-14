<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx2(): void;
}
