<?hh // strict

class Foo {
  public static function fooMeth(): void {}
}

function test_method_call(): void {
  $foo = Foo::class;

  $foo |> $$::fooMeth(); // this is a legal call
  $foo |> $$::doesNotExist(); // ...and this one doesn't exist
}
