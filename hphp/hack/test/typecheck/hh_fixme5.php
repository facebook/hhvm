<?hh // strict

class Bar {}
class Foo {}

abstract class C {
  abstract public function f(): Bar;
}

/* HH_FIXME[4110] */ abstract class D extends C {
  abstract public function f(): Foo;
}
