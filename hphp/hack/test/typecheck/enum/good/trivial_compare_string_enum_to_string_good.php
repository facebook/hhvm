<?hh

enum A: string as string {
  X = "X";
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
