<?hh

abstract class FooParent {
  abstract public function explicitCapabilityMethods()[globals, write_props]: int;
}

class Foo extends FooParent {}
  //                  ^ at-caret
