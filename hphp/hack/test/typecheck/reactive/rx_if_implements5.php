<?hh // strict

interface Rx1 {}

abstract class A {
  // ERROR: cannot combine uncondition and conditional rx attributes
  <<__Rx, __RxIfImplements(Rx1::class)>>
  public function mayberx1(): void {
    $this->mayberx2();
  }
}
