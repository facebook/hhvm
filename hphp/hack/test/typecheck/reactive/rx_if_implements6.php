<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: cannot combine uncondition and conditional rx attributes
  <<__RxShallow, __RxIfImplements(Rx1::class)>>
  public abstract function mayberx2(): void;
}
