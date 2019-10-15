<?hh

record A {
}

record B extends A {
  x: int,
}
<<__EntryPoint>> function main(): void {
$b = B['x' => 10];
}
