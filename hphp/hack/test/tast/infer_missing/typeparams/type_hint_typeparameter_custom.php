<?hh //partial

class C<T> {
  public T $y;
  public function __construct(T $x) {
    $this->y = $x;
  }
}

function foo(C $x) {
  $x->y = 4;
}
