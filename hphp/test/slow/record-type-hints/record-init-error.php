<?hh
record Foo {
  x: int,
  y: string,
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 10, 'y' => 20];
}
