<?hh

final record Foo {
  x: int,
  y: string,
}

$foo = Foo['y' => 'abc'];
