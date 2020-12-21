<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
class A {
  public function __construct(public varray<int> $v) {
  }
}

<<__Rx>>
function f(A $a)[]: void {
  $a->v[] = 1;
}
