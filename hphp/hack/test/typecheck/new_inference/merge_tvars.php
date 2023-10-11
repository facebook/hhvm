<?hh

class A<+T1, -T2 as T1> {
  public function __construct(private T2 $x) {}
  public function get(): ?T1 {
    return null;
  }
}

function test(): void {
  $x = new A(0); // x : A<v1, v2>, int <: -v2 <: +v1
  take_nullable_arraykey($x->get()); // +v1 <: arraykey
}

function take_nullable_arraykey(?arraykey $x): void {}
