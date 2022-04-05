<?hh

class Foo {
  public function __construct(int $x): void {}
}

function call_it(): void {
  new Foo(42);
  //      ^ hover-at-caret
}
