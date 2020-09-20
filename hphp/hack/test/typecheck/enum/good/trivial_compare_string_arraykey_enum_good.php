<?hh

enum A: string as arraykey {
  X = "X";
}

function f1(int $x): bool {
  return A::X === $x;
}

function f2(int $x): bool {
  return $x === A::X;
}

function f3(string $x): bool {
  return A::X === $x;
}

function f4(string $x): bool {
  return $x === A::X;
}

function f5(int $x): bool {
  return A::X !== $x;
}

function f6(int $x): bool {
  return $x !== A::X;
}

function f7(string $x): bool {
  return A::X !== $x;
}

function f8(string $x): bool {
  return $x !== A::X;
}
