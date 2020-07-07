<?hh

enum A: int as arraykey {
  X = 1;
}

function f1(float $x): bool {
  return A::X === $x;
}

function f2(float $x): bool {
  return $x === A::X;
}

function f3(float $x): bool {
  return A::X !== $x;
}

function f4(float $x): bool {
  return $x !== A::X;
}
