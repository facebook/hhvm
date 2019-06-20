<?hh

record Foo {
  x: int,
}

$foo = Foo['x' => 42];
unset($foo['x']);
