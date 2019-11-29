<?hh

record A {
  int x;
}

record B {
  ?A y;
}
<<__EntryPoint>> function main(): void {
$a = A['x' => 10];
$b = B['y' => null];
$b['y'] = $a;
$z = $b['y']['x'];
var_dump($z);
}
