<?hh // strict

interface I<T> {
  public function get(): T;
}

class A implements I<int>, I<num> {
  public function get(): int {
    return 0;
  }
}

trait T {
  require extends A;
}

class B extends A {
  use T;
  // Using T here is not an error, but a bug in Decl_linearize caused us to
  // consider it to be one. When checking that B satisfies its requirements,
  // instead of verifying only that B extends A, we verified that B was
  // compatibile with A and *also* all of A's ancestors (I<int> and I<num>).
  // When a class like A implements a generic interface with multiple
  // parameterizations, we arbitrarily select the first as canonical, so A and B
  // implement I<int> and NOT I<num> from the perspective of the typechecker.
  // Since I is invariant in its type parameter, when we attempt to verify that
  // B <: I<num>, we emit an error.

  // We should probably not allow classes like A, but we seem to have many in
  // practice.
}
