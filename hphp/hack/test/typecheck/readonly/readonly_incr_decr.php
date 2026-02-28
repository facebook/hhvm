<?hh

class Ref<T> {
  public function __construct(public T $value) {}
}

function foo(readonly Ref<int> $r): void {
  $r->value++;
  $r->value--;
  ++$r->value;
  --$r->value;

  $v = 0;
  $v++; // OK
}
