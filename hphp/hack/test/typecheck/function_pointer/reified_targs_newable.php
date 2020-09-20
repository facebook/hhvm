<?hh

<<__ConsistentConstruct>>
abstract class A {}

class B extends A {}

function foo<<<__Newable>> reify T as A>(): dynamic {
  return new T();
}

function test(): void {
  $x = foo<B>; // Fine

  $x = foo<A>; // Not fine, because A is abstract
}
