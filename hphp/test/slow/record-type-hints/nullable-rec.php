<?hh

final record A {
  x: int,
}

final record B {
  y: ?A,
}

$a = A['x' => 10];
$b = B['y' => null];
$b['y'] = $a;
$z = $b['y']['x'];
var_dump($z);
