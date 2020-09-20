<?hh // partial

class A {
}

<<__Rx>>
function f((function(Mutable<A>): void) $a): void {
}
