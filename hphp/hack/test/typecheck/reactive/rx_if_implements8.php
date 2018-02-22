<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: incorrect attribute parameter type
  <<__RxIfImplements("Rx1::class")>>
  public abstract function mayberx2(): void;
}
