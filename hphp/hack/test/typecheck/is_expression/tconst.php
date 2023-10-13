<?hh

abstract class C {
  const type T = arraykey;

  public static function isT(mixed $x): void {
    if ($x is this::T) {
      expect_arraykey($x);
    }
  }
}

abstract class D {
  abstract const type T as arraykey;

  public static function isT(mixed $x): void {
    if ($x is this::T) {
      expect_arraykey($x);
    }
  }
}

function expect_arraykey(arraykey $x): void {}
