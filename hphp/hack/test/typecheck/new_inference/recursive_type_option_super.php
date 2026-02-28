<?hh

class A<T> {
  public function __construct(public T $x) {}
  public function foo(?T $x): void {}
}

function test(): void {
  $x = new A(0); // new tvar v, $x : A<v>
  $x->foo($x->x); // v <: ?v --> should be valid
}
