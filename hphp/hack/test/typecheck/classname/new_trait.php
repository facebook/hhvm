<?hh

trait Foo {}

function f(classname<Foo> $x) {
  new $x();
}
