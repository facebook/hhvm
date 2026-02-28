<?hh

function get(): int {
  return 1;
}

class Foo {
  public varray<int> $vec = vec[get(), 2];
}
