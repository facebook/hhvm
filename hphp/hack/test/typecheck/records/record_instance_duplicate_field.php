<?hh

record A {
  int x;
  string y;
}

function new_a(): A {
  return A['x' => 1, 'y' => 'stuff', 'x' => 2];
}

function new_a_recordarray(): A {
  return A@['x' => 1, 'y' => 'stuff', 'x' => 2];
}
