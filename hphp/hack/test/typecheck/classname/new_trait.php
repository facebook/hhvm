<?hh

trait Foo {}

function f(class<Foo> $x): void {
  new $x();
}
