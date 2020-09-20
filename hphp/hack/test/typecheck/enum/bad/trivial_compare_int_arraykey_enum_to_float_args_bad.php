<?hh

enum A: int {
  X = 1;
}

function f1(float $x, A $y): bool {
  return $x === $y;
}

function f2(float $x, A $y): bool {
  return $y === $x;
}

function f3(float $x, A $y): bool {
  return $x !== $y;
}

function f4(float $x, A $y): bool {
  return $y !== $x;
}
