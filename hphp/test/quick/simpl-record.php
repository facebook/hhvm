<?hh
final record Foo {
  x: int,
  y: int,
}

<<__EntryPoint>> function main():void {
$foo = Foo['x' => 10, 'y' => 20];
$foo['x'] = 100;
$bar = $foo['x'];
$baz = $foo['y'];
var_dump($bar);
var_dump($baz);
}
