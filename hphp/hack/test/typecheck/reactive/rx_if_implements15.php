<?hh // strict

interface Rx1 {
}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public function mayberx(): int {
    // ok - condition matches
    return $this->mayberx2();
  }

  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public abstract function mayberx2(): int;
}
