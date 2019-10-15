<?hh

record Foo {
  x:int,
}
<<__EntryPoint>> function main(): void {
$f = Foo['x' => 1];
$a = 10;
$f['x'] =& $a;
$a = 100;
var_dump($f['x']);
}
