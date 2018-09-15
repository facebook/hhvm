<?hh // strict

interface Rx1 {}

interface I {
  <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public function condlocal(): int;
}

abstract class A {
  <<__Rx>>
  public function rx(I $i): int {
    // ERROR
    return $i->condlocal();
  }
}
