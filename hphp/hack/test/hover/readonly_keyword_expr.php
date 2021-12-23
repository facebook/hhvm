<?hh

class Foo {
  public function __construct(public int $prop) {}
}

function bar(): void {
  $f = readonly new Foo();
  //      ^ hover-at-caret
}
