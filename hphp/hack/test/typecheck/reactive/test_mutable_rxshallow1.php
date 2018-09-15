<?hh // strict

class C {
  public function __construct(public int $v) {}
}

<<__RxShallow>>
function f(C $c): void {
  // not OK - RxShallow functions behave like reactive
  $c->v = 5;
}
