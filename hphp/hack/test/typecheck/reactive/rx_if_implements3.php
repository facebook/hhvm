<?hh // strict

interface Rx1 {}

abstract class A {
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function mayberx1(): void {
    $this->mayberx2();
  }

  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx2(): void;

  <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx3(): void;
}
