<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as arraykey> {
  public static function nongeneric(T $x): T {
    return $x;
  }
  public static function generic<Tu>(T $x, Tu $y): (T, Tu) {
    return tuple($x, $y);
  }
}

function testdirect(string $s, int $i, bool $b): (string, (int, bool)) {
  return tuple(C::nongeneric($s), C::generic($i, $b));
}

function testindirect(string $s, int $i, bool $b): (string, (int, bool)) {
  $f = C::nongeneric<>;
  $g = C::generic<>;
  return tuple($f($s), $g($i, $b));
}
