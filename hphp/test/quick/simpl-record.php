<?hh
final record Foo {
  x: int,
  y: int,
}

<<__EntryPoint>> function main():void {
  $foo = Foo['x' => 10, 'y' => 20];
  $bar = $foo;
  $foo['x'] = 100;
  var_dump($foo['x']);
  var_dump($foo['y']);
  var_dump($bar['x']);
}
