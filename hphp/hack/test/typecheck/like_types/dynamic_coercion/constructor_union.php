<?hh // partial

final class C {
  public function __construct(int $i) {}
}

final class D {
  public function __construct(string $s) {}
}

function f(bool $b, dynamic $d): void {
  $x = $b ? C::class : D::class;
  new $x($d);
}
