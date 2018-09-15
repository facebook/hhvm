<?hh // strict
class Bar {}
class Baz extends Bar {}
class Foo {
  const int x = 5;
  const type T as Bar = Baz;
}

function test () : void {
  $y = Foo::x;
}

function const_in_shape () : void {
  $x = shape(Foo::x => 7);
  // UNSAFE
}
