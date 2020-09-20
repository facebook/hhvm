<?hh

enum A: int {
  X = 1;
}

function f1(bool $x): bool {
  return A::X === $x;
}

function f2(bool $x): bool {
  return $x === A::X;
}

function f3(bool $x): bool {
  return A::X !== $x;
}

function f4(bool $x): bool {
  return $x !== A::X;
}
