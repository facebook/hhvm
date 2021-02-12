<?hh // strict
class Foo {
  public function __construct(public int $value) {}
}


function test(Foo $x)[]: void {
  $x->value = 5;
}
