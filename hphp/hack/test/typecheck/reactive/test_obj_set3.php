<?hh // strict
class Foo {
  public function __construct(public Vector<int> $value) {}
}


function test(Foo $x)[]: void {
  $x->value[] = 5;
}
