<?hh // strict

class B { }
class C<T as B> {
  public function __construct(private T $item) { }
  public function get():T { return $this->item; }
}

function test_flow<T>(mixed $m, bool $flag):B {
  if ($flag) {
    $b = new B();
  } else {
    invariant($m is C<_>, "C");
    $b = $m->get();
  }
  // we want $b : T#1 and T#1 <: I to survive exit from the else branch above
  return $b;
}
