<?hh // strict

interface A {}
interface RxA extends A {}

<<__Rx>>
function f<T>(T $a, T $b): T {
  return $a;
}

class C1 {
  <<__Rx, __AtMostRxAsArgs>>
  public function f(
    <<__OnlyRxIfImpl(RxA::class)>>A $a,
    <<__OnlyRxIfImpl(RxA::class)>>A $b,
  ): void {
    // OK
    $c = f($a, $b);
    $c1 = varray[$a, $b];
  }
}
