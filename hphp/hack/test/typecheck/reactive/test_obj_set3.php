<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class Foo {
  public function __construct(public Vector<int> $value) {}
}

<<__Rx>>
function test(Foo $x)[rx]: void {
  $x->value[] = 5;
}
