<?hh

class C {
  public static int $sprop = 4;
}
class D {
  public static string $sprop = "str";
}

function f(bool $b, dynamic $d): void {
  $x = $b ? C::class : D::class;
  hh_show($x);
  $x::$sprop = $d;
}
