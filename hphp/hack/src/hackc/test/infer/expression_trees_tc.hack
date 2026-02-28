// Typecheck Helpers

interface Spliceable<TVisitor, TResult, +TInfer> {
}

class A implements Spliceable<A, A, mixed> {
  public function splice(mixed $a, string $b, A $c): void { }
  static public function makeTree<T>(mixed $a, mixed $b, mixed $c): A { invariant_violation("bad"); }
  static public function lift<T>(A $x): A {
    return $x;
  }
}
