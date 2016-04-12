<?hh // strict

class Foo {}
class Bar extends Foo {}
class Baz extends Foo {}

function f(mixed $x): Foo {
  $classes = Vector {Bar::class, Baz::class};
  if ($x instanceof $classes[0]) {
    hh_show($x);
    return $x;
  }
  invariant_violation();
}
