<?hh

class Foo {
  public function __construct(public int $prop) {}
}

function bar(): readonly Foo {
  //              ^ hover-at-caret
  throw new Exception();
}
