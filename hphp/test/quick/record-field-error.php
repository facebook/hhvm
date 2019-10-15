<?hh
record Foo {
  x:int,
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 1];
$y = $foo['y'];
}
