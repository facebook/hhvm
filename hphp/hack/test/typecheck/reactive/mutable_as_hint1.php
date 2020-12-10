<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {}

// ERROR
<<__Rx>>
function f(Mutable<A> $a): void {
}
