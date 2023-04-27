<?hh

class FooParent {
  public function __construct() {}
}

class Foo extends FooParent {
}

function call_construct(): void {
  new Foo();
  //  ^ hover-at-caret
}
