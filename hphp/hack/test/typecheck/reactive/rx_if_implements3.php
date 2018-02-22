<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxIfImplements(Rx1::class)>>
  public function mayberx1(): void {
    $this->mayberx2();
  }

  <<__RxShallowIfImplements(Rx1::class)>>
  public abstract function mayberx2(): void;

  <<__RxLocalIfImplements(Rx1::class)>>
  public abstract function mayberx3(): void;
}
