<?hh

function f(inout C $c): void {}

class C {
  public function __construct(private C $x) {}

  public function bar(): void {
    f(inout $this->x->x);
  }
}
