<?hh

abstract final class Foo {}

function f(): classname<Foo> {
  return Foo::class;
}
