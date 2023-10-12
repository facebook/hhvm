<?hh // strict

class Foo {
  public static int $myInt = 0;
}

function takes_an_int(int $int): void {}

function test_member(): void {
  $foo = Foo::class;

  $foo |> $$::$myInt; // make sure we can access static members...
  $foo |> takes_an_int($$::$myInt); // and that they have the right type...
  $foo |> $$::$notAMember; // but this fails.
}
