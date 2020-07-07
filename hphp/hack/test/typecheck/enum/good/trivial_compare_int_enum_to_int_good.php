<?hh

enum A: int as int {
  X = 1;
}

function f1(int $x): bool {
  return A::X === $x;
}

function f2(int $x): bool {
  return A::X !== $x;
}

function f3(int $x): bool {
  return $x === A::X;
}

function f4(int $x): bool {
  return $x !== A::X;
}
