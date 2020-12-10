<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface A {}
interface RxA {}

class C1 {
  <<__Rx, __AtMostRxAsArgs>>
  public function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
  }
}

class C2 {
  // OK
  <<__Rx>>
  public function f(A $a): void {
  }
}
