<?hh // strict

class Foo {
  public function __construct(public Vector<int> $value) {}
}

<<__Rx>>
function test(Foo $x): void {
  $x->value[] = 5;
}
