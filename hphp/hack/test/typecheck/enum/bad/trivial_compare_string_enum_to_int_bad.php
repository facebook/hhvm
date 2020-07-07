<?hh

enum A: string as string {
  X = "X";
}

function f(int $x): bool {
  return $x === A::X;
}

function g(int $x): bool {
  return $x !== A::X;
}
