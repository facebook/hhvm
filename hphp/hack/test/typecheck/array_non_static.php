<?hh // strict

function get(): int {
  return 1;
}

class Foo {
  public array<int> $vec = varray[get(), 2];
}
