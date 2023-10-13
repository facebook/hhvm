<?hh

function f(): int {
  return 10;
}

class A {
  const int X = 10;
  const int Y = -(X + (10 - 2 * X));

  const int Z = 10 + f();
}
