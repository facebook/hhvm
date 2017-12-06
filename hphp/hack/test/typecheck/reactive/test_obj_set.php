<?hh // strict

class Foo {
  public function __construct(public int $value) {}
}

<<__Rx>>
function test(Foo $x): void {
  $x->value = 5;
}
