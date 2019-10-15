<?hh

record Foo {
  x: int,
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 42];
unset($foo['x']);
}
