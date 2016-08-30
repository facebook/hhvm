<?hh

class A {}

class B<T> {}

function bar(A $a, B<A> $b) : void {
}

function foo(A $a, B<A> $b) : void {
  bar($a, $b);
}
