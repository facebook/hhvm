<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: incorrect attribute parameter count
  <<__RxIfImplements(Rx1::class, 42)>>
  public abstract function mayberx2(): void;
}
