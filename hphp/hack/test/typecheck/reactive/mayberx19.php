<?hh // strict

interface A {}
interface RxA extends A {}

class C1 {
  <<__Rx>>
  public function f(RxA $a): void {
  }
}

class C2 extends C1 {
  // OK (though condition is kind of useless)
  <<__Rx, __AtMostRxAsArgs>>
  public function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
  }
}
