<?hh

final record A {
}

final record B extends A {
  x: int,
}

$b = B['x' => 10];
