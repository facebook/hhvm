<?hh // strict

interface A {
}

interface RxA {
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
}

class C implements A {
}

<<__Rx>>
function b(C $c): void {
  // ERROR
  f($c);
}
