<?hh

enum A: int as arraykey {
  X = 1;
}

function f1(num $x): bool {
  return A::X === $x;
}

function f2(num $x): bool {
  return $x === A::X;
}

function f3(num $x): bool {
  return A::X !== $x;
}

function f4(num $x): bool {
  return $x !== A::X;
}
