<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface Rx1 {}

abstract class A {
  // ERROR: incorrect attribute parameter count
  <<__Rx, __OnlyRxIfImpl(Rx1::class, 42)>>
  public abstract function mayberx2(): void;
}
