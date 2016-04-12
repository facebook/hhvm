<?hh // strict

class Foo {}

abstract class F {
  abstract const type T as Foo;
  public this::T $v;
  public function __construct() {
    // UNSAFE
  }
}

function f(F $x, mixed $y): Foo {
  if ($y instanceof $x->v) {
    hh_show($y);
    return $y;
  }
  invariant_violation('');
}
