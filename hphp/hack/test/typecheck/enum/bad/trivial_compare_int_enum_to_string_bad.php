<?hh

enum A: int as int {
  X = 1;
}

function f1(string $x): bool {
  return A::X === $x;
}

function f2(string $x): bool {
  return $x === A::X;
}

function f3(string $x): bool {
  return A::X !== $x;
}

function f4(string $x): bool {
  return $x !== A::X;
}
