<?hh
class A {}

// ERROR

function f(Mutable<A> $a): void {
}
