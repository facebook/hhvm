<?hh

abstract record A {
  int x;
}

record B extends A {
  int y;
  int z = 1;
}

function foo(): void {
  $good = B['x' => 1, 'y' => 2, 'z' => 3];
  $good_default = B['x' => 1, 'y' => 2];
  $missing_x = B['y' => 1];

  $unknown_field = B['x' => 1, 'y' => 2, 'unexpected' => 3];
}
