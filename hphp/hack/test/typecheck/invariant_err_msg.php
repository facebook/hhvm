<?hh

class A {}
class B extends A {}

function foo(Vector<B> $x): Vector<A> {
  return $x;
}
