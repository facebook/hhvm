<?hh

abstract final class Foo {}

function f(classname<Foo> $x): void {
  new $x();
}
