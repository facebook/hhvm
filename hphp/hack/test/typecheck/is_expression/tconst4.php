<?hh

abstract class C {
  const type T = D::T;

  public static function isT(mixed $x): void {
    if ($x is self::T) {
      expect_arraykey($x);
    }
  }
}

abstract class D {
  const type T = E::T;
}

abstract class E {
  const type T = arraykey;
}

function expect_arraykey(arraykey $x): void {}
