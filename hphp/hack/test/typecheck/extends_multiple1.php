<?hh //strict

interface I<+T> {
  public function get(): T;
}

class A {}
class B {}

// in declaration such as this, A and B must be in common
// hierarchy, or must have a common descendant
interface IAB extends I<B>, I<A> {}
