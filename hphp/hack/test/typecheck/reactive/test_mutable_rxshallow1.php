<?hh // strict
class C {
  public function __construct(public int $v) {}
}


function f(C $c)[]: void {
  // not OK - RxShallow functions behave like reactive
  $c->v = 5;
}
