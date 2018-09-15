<?hh // strict

class Test {

  public function __construct(public int $val) {}
}

<<__Rx>>
function foo(<<__Mutable>>Test $x, Test $y): void {
  $x->val = 5;
  // error, y is not mutable
  $y->val = 4;
}
