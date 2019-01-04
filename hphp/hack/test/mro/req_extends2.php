<?hh // strict

class A<+T> {
  public function get(): T {
    // UNSAFE
  }
}

trait T {
  require extends A<mixed>;
}

// A<mixed> will occur before A<int> in the linearization of B, so we need to
// mark A<mixed> as arising from a synthesized inheritance relationship
// (require-extends) if we would like B to inherit methods from A<int> rather
// than A<mixed>.
class B extends A<int> {
  use T;
}

// The same situation as with B arises here, but with the added complication
// that the MRO source gives us no indication that A<mixed> might have arisen
// from a require-extends relationship (since the source is Parent for every
// element other than C). In order to allow inheriting methods from A<int>
// instead of A<mixed>, we mark A<mixed> as a synthesized ancestor.
class C extends B {}
