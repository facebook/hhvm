<?hh //partial

function foo($x) : int {
  if ($x is int) {
    return $x;
  }
  return (int) $x;
}

function bar($x) : int {
  return $x + 4;
}
