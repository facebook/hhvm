<?hh

record Foo {
  int x;
  string y;
}
<<__EntryPoint>> function main(): void {
$foo = Foo['y' => 'abc'];
}
