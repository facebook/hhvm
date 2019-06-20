<?hh
final record Foo {
  x:int,
}

$foo = Foo['x' => 1];
$y = $foo['y'];
