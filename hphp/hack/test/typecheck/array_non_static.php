<?hh // strict

function get(): int {
  return 1;
}

class Foo {
  public varray<int> $vec = varray[get(), 2];
}
