<?hh

class C<reify T> {
  public function __construct(private T $item) {}
  public function get(): T {
    return $this->item;
  }
}

function test1(): void {
  $f = (C<_> $x) ==> $x->get();
  $x = $f(new C<int>(3));
}

function test2(): void {
  $f = (C<_> ...$x) ==> $x[0]->get();
  $x = $f(new C<int>(3));
}
