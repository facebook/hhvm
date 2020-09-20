<?hh

abstract record A {
  int x;
}

function new_a(): A {
  return A['x' => 1];
}

