<?hh // strict

interface Rx1 {}
interface Rx2 {}

interface I {
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function condlocal(): int;
}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx2::class)>>
  public function rx(I $i): int {
    // ERROR, calling conditionally reactive method when condition
    // does not match
    return $i->condlocal();
  }
}
