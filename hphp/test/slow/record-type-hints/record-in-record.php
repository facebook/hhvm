<?hh
final record Foo {
  y: int,
}

final record Bar {
  x:Foo,
}
<<__EntryPoint>> function main(): void {
$z = Foo['y'=>42];
$a = Bar['x'=>$z];
$b = $a['x'];
$c = $b['y'];
var_dump($c);
}
