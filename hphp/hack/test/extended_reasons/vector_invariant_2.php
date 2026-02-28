<?hh

function bar(Vector<A> $x): void {}

function foo(Vector<?A> $x): Vector<A> {
 bar($x);
 return $x;
}

class A {}
