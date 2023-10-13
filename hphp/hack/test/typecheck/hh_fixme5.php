<?hh

class Bar {}
class Foo {}

abstract class C {
  abstract public function f(): Bar;
}

abstract class D extends C {
  /* HH_FIXME[4341] */
  abstract public function f(): Foo;
}
