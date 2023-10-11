<?hh

class Foo {
  const int MY_CONST = 5;
}

function baz(): int {
  return Foo::class |> $$::MY_CONST;
}
