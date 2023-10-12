<?hh // strict

class B { }
class C<T as B> {
  public function __construct(private T $item) { }
  public function get():T { return $this->item; }
}
class D<T> {
}

function test_flow<T>(mixed $m, mixed $m2, bool $flag):B {
  if ($flag) {
    $b = new B();
    invariant($m2 is D<_>, "D");
  } else {
    invariant($m is C<_>, "C");
    $b = $m->get();
  }
  // we want $b : T#1 and T#1 <: I to survive exit from the else-branch above
  // So we want fresh param in then-branch to have a different name!
  return $b;
}
