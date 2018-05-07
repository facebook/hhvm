<?hh // strict

interface A {
}

interface RxA {
}

//ERROR
<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfImpl("RxA")>>A $a): void {
}
