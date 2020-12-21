<?hh // partial
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public ?int $v;
  public function __construct(public A $obj) {}
}

<<__Rx>>
function f(<<__Mutable>>A $a)[]: void {
  // ERROR
  unset($a->obj->v);
}
