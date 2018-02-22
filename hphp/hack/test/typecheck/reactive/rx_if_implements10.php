<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxLocalIfImplements(Rx1::class)>>
  public abstract function mayberx2(): void;
}
