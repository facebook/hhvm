<?hh

abstract class A {
  abstract const type T as nonnull = int;
}

trait ReqExtA {
  require extends A;
}

abstract class B extends A {
  use ReqExtA;

  // This return statement is an error--this::T is abstract with a constraint of
  // nonnull, so we have no guarantee that subclasses will choose a type for T
  // which is a supertype of int. In a concrete class which does not otherwise
  // declare T, we will select the default type of int, but B is not a concrete
  // class. A bug in shallow decl caused us to assign T the default type of int
  // in B (even though B is abstract) because of the require-extends A clause.
  // This caused the typechecker to fail to emit any errors in this file.
  public static function get(): this::T {
    return 0; // error
  }
}

class C extends B {
}

function test_b(): int {
  return B::get(); // error
}

function test_c(): int {
  return C::get(); // ok
}
