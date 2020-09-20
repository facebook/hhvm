<?hh // strict

interface A {
}

interface RxA {
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
}

class C implements A, RxA {
}

<<__Rx>>
function b(C $c): void {
  // OK
  f($c);
}

<<__Rx>>
function b1<T>(T $c): void where T as A, T as RxA {
  // OK
  f($c);
}
