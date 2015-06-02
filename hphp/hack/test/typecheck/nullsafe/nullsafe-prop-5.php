<?hh

class C {
  public int $x = 123;
}

function test(?C $c): void {
  unset($c?->x);
}
