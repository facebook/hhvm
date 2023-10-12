<?hh // strict

abstract class C {
  const type T = arraykey;

  public static function isT(mixed $x): void {
    if ($x is self::T) {
      expect_arraykey($x);
    }
  }
}

function expect_arraykey(arraykey $x): void {}
