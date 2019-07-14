<?hh

final record A {
}

final record B extends A {
  x: int,
}
<<__EntryPoint>> function main(): void {
$b = B['x' => 10];
}
