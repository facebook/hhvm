<?hh

record Foo {
  x: int = 42,
  z: ?bool = 1,
  y: string,
}
<<__EntryPoint>> function main(): void {
$a = Foo['z' => null, 'y' => 'abc'];
var_dump($a['x']);

$b = Foo['y' => 'def'];
}
