<?hh

enum X: int as int {
  A = 1;
  B = 2;
}

function f1(num $n): bool {
  return ($n === X::A);
}

function f2(num $n): bool {
  return (X::A === $n);
}
