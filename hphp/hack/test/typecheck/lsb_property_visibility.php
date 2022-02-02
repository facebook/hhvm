<?hh

class Foo {
  <<__LSB>>
  private static int $bar = 1;
}

class Qux extends Foo {}

function baz(): void {
  Foo::$bar;
  Qux::$bar;
}
