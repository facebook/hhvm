<?hh

class A<T> {
  public function __construct(public T $x) {}
  public function get(): T { return $this->x; }
  public function put(T $x): void {}
}

function test(bool $cond): void {
  $x = new A(0); // x : A<v1>, int <: v1
  $a = ($cond ? $x->get() : ($cond ? true : null)); // a : ?(bool | v1)
  $y = new A(""); // y : A<v2>, string <: v2
  $b = ($cond ? $y->get() : ($cond ? 0.1 : null)); // b : ?(float | v2)
  $x->put($a); // ?(bool | v1) <: v2
  $y->put($b); // ?(float | v2) <: v1
}
