<?hh // strict

interface A {
  public function f(): int;
}

interface RxA {
  <<__Rx>>
  public function f(): int;
}

<<__Rx, __AtMostRxAsArgs>>
function a(<<__OnlyRxIfImpl(RxA::class)>>A $a): int {
  return b($a);
}

<<__Rx, __AtMostRxAsArgs>>
function b(<<__OnlyRxIfImpl(RxA::class)>>A $a): int {
  // ERROR: reactive lambda cannot call conditionally reactive function
  $x = <<__Rx>> () ==> $a->f();
  return $x();
}
