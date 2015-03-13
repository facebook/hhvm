<?hh // strict

class C {
  public int $x = 123;
}

function test(?C $c): ?int {
  return $c?->x;
}
