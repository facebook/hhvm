<?hh // strict

function get(): int {
  return 1;
}

class Foo {
  public array<int> $vec = array(get(), 2);
}
