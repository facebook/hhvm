<?hh

trait FooTrait {
  abstract public function bar(): void;
  abstract public function baz(): void;
}

class Foo {
//      ^ at-caret
  use FooTrait;
}
