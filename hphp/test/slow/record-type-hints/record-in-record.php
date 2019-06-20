<?hh
final record Foo {
  y: int,
}

final record Bar {
  x:Foo,
}

$z = Foo['y'=>42];
$a = Bar['x'=>$z];
$b = $a['x'];
$c = $b['y'];
var_dump($c);
