<?hh

trait Foo {}

function f(classname<Foo> $x): void {
  new $x();
}
