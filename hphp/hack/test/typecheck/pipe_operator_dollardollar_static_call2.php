<?hh // strict

class Foo {
  public static function fooMeth(): void {}
}

class Bar {
  public static function barMeth(): void {}
}

function gives_a_bar(classname<Foo> $foo): classname<Bar> {
  return Bar::class;
}

function test_secondary_call(): void {
  // Make sure $$ always points to the right type
  Foo::class |> gives_a_bar($$) |> $$::barMeth();
}
