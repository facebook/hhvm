<?hh

class C {
  public X $x;
  public function __construct() {
    $this->x = new X();
  }
}

class X {
  public int $prop = 1;
  public function method(?int $v): int {
    return is_null($v) ? 1 : $v;
  }
}

function test(?C $c): ?int {
  return $c?->x?->method($c?->x?->prop);
}
