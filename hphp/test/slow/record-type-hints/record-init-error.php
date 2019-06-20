<?hh
record Foo {
  x: int,
  y: string,
}

$foo = Foo['x' => 10, 'y' => 20];
