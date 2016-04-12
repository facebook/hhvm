<?hh // strict

function get(): int {
  return 1;
}

class Foo {
  public (int, int, int) $list_tuple = list(get(), 3, 4);
}
