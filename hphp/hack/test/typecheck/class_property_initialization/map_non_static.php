<?hh // strict

function get(): string {
  return 'hello';
}

class Foo {
  public Map<int, string> $vec = Map { 3 => get(), 4 => 'world' };
}
