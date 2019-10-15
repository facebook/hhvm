<?hh
record Foo {
  x: int,
  y: int,
  z: mixed,
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 10, 'y' => 20, 'z' => true];
$foo['x'] = 'hello';
$bar = $foo['x'];
$baz = $foo['y'];
var_dump($bar);
var_dump($baz);
}
