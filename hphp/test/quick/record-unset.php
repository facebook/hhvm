<?hh

record Foo {
  int x;
}
<<__EntryPoint>> function main(): void {
$foo = Foo['x' => 42];
unset($foo['x']);
}
