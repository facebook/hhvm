<?hh // strict

class C {
  public int $x = 123;
}

function test(?C $c): void {
  $c?->x++;
}
