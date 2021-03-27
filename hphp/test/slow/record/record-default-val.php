<?hh

record Foo {
  int x = 42;
  ?bool z = 1;
  string y;
}
<<__EntryPoint>> function main(): void {
$a = Foo['z' => null, 'y' => 'abc'];
var_dump($a['x']);

$b = Foo['y' => 'def'];
}
