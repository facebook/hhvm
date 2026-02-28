<?hh

abstract class FooParent {
  abstract public function bar(): void;
  abstract public function baz(): void;
}

class Foo extends FooParent {}
  //                  ^ at-caret
