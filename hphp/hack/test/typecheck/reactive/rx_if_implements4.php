<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: cannot combine __Rx and __RxIfImplements
  <<__Rx, __RxIfImplements(Rx1::class)>>
  public function mayberx1(): void {
  }

  <<__RxShallowIfImplements(Rx1::class)>>
  public abstract function mayberx2(): void;
}
