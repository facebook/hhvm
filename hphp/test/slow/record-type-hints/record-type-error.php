<?hh
final record Foo {
  x: int,
  y: int,
  z: mixed,
}

$foo = Foo['x' => 10, 'y' => 20, 'z' => true];
$foo['x'] = 'hello';
$bar = $foo['x'];
$baz = $foo['y'];
var_dump($bar);
var_dump($baz);
