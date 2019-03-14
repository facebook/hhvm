<?hh // partial

abstract final class Foo {}

function f(classname<Foo> $x) {
  new $x();
}
