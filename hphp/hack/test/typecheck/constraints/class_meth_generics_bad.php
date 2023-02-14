<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T as arraykey> {
  public static function nongeneric(T $x): T {
    return $x;
  }
  public static function generic<Tu>(T $x, Tu $y): (T, Tu) {
    return tuple($x, $y);
  }
  public static function genericConstrained<Tu as arraykey>(
    T $x,
    Tu $y,
  ): (T, Tu) {
    return tuple($x, $y);
  }
}

function testindirect1(bool $b): bool {
  $f = C::nongeneric<>;
  $fr = $f($b);
  return $fr;
}

function testindirect2(float $f, vec<int> $v): (float, vec<int>) {
  $g = C::generic<>;
  $gr = $g($f, $v);
  return $gr;
}
function testindirect3(float $f, vec<int> $v): (float, vec<int>) {
  $g = C::genericConstrained<>;
  $gr = $g($f, $v);
  return $gr;
}
