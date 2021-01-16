<?hh
class A {}

// ERROR
<<__Rx>>
function f(Mutable<A> $a): void {
}
