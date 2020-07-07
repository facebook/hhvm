<?hh

enum X: int {
  A = 0;
  B = 1;
}

function f1(X $x): bool {
  return (!$x === X::A);
}

function f2(X $x): bool {
  return ($x === !X::A);
}
