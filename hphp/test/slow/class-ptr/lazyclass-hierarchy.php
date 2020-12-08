<?hh

class Foo {}
class Bar extends Foo {}

function foo<T>(mixed $c, classname<T>$base) {
  return $c is string && \is_a($c, $base, true);
}

<<__EntryPoint>>
function main() {
  var_dump(foo(Foo::class, Bar::class));
  var_dump(foo(Bar::class, Foo::class));
  // No class called Baz
  var_dump(foo(Baz::class, Foo::class));
  var_dump(foo(Foo::class, Baz::class));
}
