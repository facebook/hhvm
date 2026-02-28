<?hh

abstract class FooParent {
  abstract public function somePureMethod()[]: int;
}

class Foo extends FooParent {}
  //                  ^ at-caret
