<?hh // strict

final class D {
  <<__Memoize>>
  public static function baz(mixed $x, inout mixed $y, mixed $z): void {}
}
