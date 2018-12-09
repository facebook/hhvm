<?hh // strict

interface A {
}

interface RxA {
}

//ERROR
<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl("RxA")>>A $a): void {
}
