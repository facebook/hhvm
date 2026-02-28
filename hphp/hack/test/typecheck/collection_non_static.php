<?hh

function get(): int {
  return 1;
}

class Foo {
  public Vector<int> $vec = Vector { get(), 2 };
}
