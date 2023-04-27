 <?hh

function takes_an_int(int $int): void {}

function pipe_param_call(int $x): void {
  $x |> takes_an_int($$);
}

function pipe_local_call(): void {
  $x = 1;
  $x |> takes_an_int($$);
}

class Foo {
  public static int $myInt = 0;
}

function pipe_taccess(): void {
  $foo = Foo::class;
  $foo |> $$::$myInt;
}

function pipe_taccess_call(): void {
  $foo |> takes_an_int($$::$myInt);
}
