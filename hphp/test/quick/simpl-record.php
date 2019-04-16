<?hh
record Foo {
  x: int,
  y: int,
}

$foo = Foo['x' => 10, 'y' => 20];
$foo['x'] = 100;
$bar = $foo['x'];
$baz = $foo['y'];
var_dump($bar);
var_dump($baz);
