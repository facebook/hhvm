<?hh

abstract final class Foo {}

function f(class<Foo> $x): void {
  new $x();
}
