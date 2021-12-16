<?hh

class Foo {
  public function __construct(public int $prop) {}
}

function bar(readonly Foo $f): void {}
//              ^ hover-at-caret
