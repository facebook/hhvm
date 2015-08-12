<?hh // strict

class A {}
class B extends A {}

function foo(Set<B> $x): Set<A> {
  return $x;
}
