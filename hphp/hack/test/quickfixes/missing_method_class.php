<?hh

abstract class FooParent {
  abstract public function bar(): void;
}

class Foo extends FooParent {}
