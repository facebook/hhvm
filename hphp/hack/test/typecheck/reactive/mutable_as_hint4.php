<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
}

<<__Rx>>
function f((function(Mutable<A>): void) $a): void {
}
