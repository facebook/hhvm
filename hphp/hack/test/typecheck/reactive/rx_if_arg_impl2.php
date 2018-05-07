<?hh // strict

interface A {
}

interface RxA {
}

<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
}

class C implements A {
}

<<__Rx>>
function b(C $c): void {
  // ERROR
  f($c);
}
