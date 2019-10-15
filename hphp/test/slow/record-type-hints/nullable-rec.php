<?hh

record A {
  x: int,
}

record B {
  y: ?A,
}
<<__EntryPoint>> function main(): void {
$a = A['x' => 10];
$b = B['y' => null];
$b['y'] = $a;
$z = $b['y']['x'];
var_dump($z);
}
