<?hh

abstract record A {
  x: int,
}

function new_a(): A {
  return A['x' => 1];
}

function new_a_recordarray(): mixed {
  return A@['x' => 1];
}
