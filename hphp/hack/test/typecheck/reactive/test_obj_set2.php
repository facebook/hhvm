<?hh // strict
class Foo {
  public function __construct(public string $value) {}
}


function test(Foo $x)[]: void {
  $x->value .= 5;
}
