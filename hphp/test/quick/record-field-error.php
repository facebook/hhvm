<?hh
record Foo {
  int x;
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 1];
$y = $foo['y'];
}
