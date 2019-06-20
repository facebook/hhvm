<?hh
record Foo {
  y: int,
}

record Bar {
  x:Foo,
}

$z = Foo['y'=>42];
$a = Bar['x'=>$z];
$b = $a['x'];
$c = $b['y'];
var_dump($c);
