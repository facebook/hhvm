<?hh // strict

interface A {}
interface RxA {}

class C1 {
  <<__Rx>>
  public function f(A $a): void {
  }
}

class C2 extends C1 {
  // ERROR: invariant f body is reactive iff $a instanceof RxA can be violated
  <<__Rx, __AtMostRxAsArgs>>
  public function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
  }
}
