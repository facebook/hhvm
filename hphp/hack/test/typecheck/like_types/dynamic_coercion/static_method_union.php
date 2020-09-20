<?hh

class C {
  public static function f(int $arg): void {}
}
class D {
  public static function f(string $arg): void {}
}

function f(bool $b, dynamic $d): void {
  $x = $b ? C::class : D::class;
  $x::f($d);
}
