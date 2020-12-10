<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface Rx1 {}

interface I {
  public function condlocal(): int;
}

abstract class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx1::class)>>
  public function rx(I $i): int {
    // ERROR
    return $i->condlocal();
  }
}
