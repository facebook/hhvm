<?hh // strict

class A {}

// ERROR
<<__Pure>>
function f(Mutable<A> $a): void {
}
