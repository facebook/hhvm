<?hh // strict

class Foo {
  public static int $myInt = 0;
}

function takes_a_string(string $string): void {}

function test_member_type(): void {
  $foo = Foo::class;

  $foo |> takes_a_string($$::$myInt);
}
